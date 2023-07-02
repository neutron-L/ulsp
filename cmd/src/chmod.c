#include <assert.h>
#include <locale.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <dirent.h>

#include "apue.h"
#include "system.h"

#define PROGRAM_NAME "chmod"

enum
{
    NO_PRESERVE_ROOT = CHAR_MAX + 1,
    PRESERVE_ROOT,
    REFERENCE
};

static struct option const long_options[] =
    {
        {"changes", no_argument, NULL, 'c'},
        {"silent", no_argument, NULL, 'f'},
        {"quiet", no_argument, NULL, 'f'},
        {"verbose", no_argument, NULL, 'v'},
        {"no-preserve-root", no_argument, NULL, NO_PRESERVE_ROOT},
        {"preserve-root", no_argument, NULL, PRESERVE_ROOT},
        {"reference", required_argument, NULL, REFERENCE},
        {"recursive", no_argument, NULL, 'R'},
        {GETOPT_HELP_OPTION_DECL},
        {GETOPT_VERSION_OPTION_DECL},
        {NULL, 0, NULL, 0}};

static bool cflag, vflag, Rflag, has_parse_mode;
static const char *rfile;
static char *mode_list;
static int mode_len, mode_alloc;
static mode_t mode;

static struct stat statbuf;

static char who[] = {'u', 'g', 'o'};
static char mc[] = {'r', 'w', 'x', 's'};
static mode_t modes[][4] = {{S_IRUSR, S_IWUSR, S_IXUSR, S_ISUID}, {S_IRGRP, S_IWGRP, S_IXGRP, S_ISGID}, {S_IROTH, S_IWOTH, S_IXOTH}};

mode_t MODE_BIT = (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH | S_ISUID | S_ISGID);

static inline int
get_idx(char *ar, int n, char c)
{
    for (int i = 0; i < n; ++i)
        if (ar[i] == c)
            return i;
    return -1;
}

static inline bool
search(char *st, char *ed, char c)
{
    while (st != ed && *st != c)
        ++st;
    return st != ed;
}

#define CLEAR_MODE_BIT(m) ((m) & (~MODE_BIT))
#define GET_MODE_BIT(m) ((m)&MODE_BIT)
#define ISOCTAL(x) ((x) >= '0' && (x) < '8')
#define ISUSR(c) ((c) == 'u' || (c) == 'g' || (c) == 'o' || (c) == 'a')
#define ISRWX(c) ((c) == 'r' || (c) == 'w' || (c) == 'x' || (c) == 's')
#define ISOP(c) ((c) == '+' || (c) == '-' || (c) == '=')
static void
usage(int status)
{
    if (status == EXIT_SUCCESS)
    {
        printf("Usage: chmod [OPTION]... MODE[,MODE]... FILE...\n");
        printf("  or:  chmod [OPTION]... OCTAL-MODE FILE...\n");
        printf("  or:  chmod [OPTION]... --reference=RFILE FILE...\n");
        printf("Change the mode of each FILE to MODE.\n");
        printf("With --reference, change the mode of each FILE to that of RFILE.\n");
        printf("\n");
        printf("  -c, --changes          like verbose but report only when a change is made\n");
        printf("  -f, --silent, --quiet  suppress most error messages\n");
        printf("  -v, --verbose          output a diagnostic for every file processed\n");
        printf("      --no-preserve-root  do not treat '/' specially (the default)\n");
        printf("      --preserve-root    fail to operate recursively on '/'\n");
        printf("      --reference=RFILE  use RFILE's mode instead of MODE values\n");
        printf("  -R, --recursive        change files and directories recursively\n");
        printf("      --help     display this help and exit\n");
        printf("      --version  output version information and exit\n");
        printf("\n");
        printf("Each MODE is of the form '[ugoa]*([-+=]([rwxXst]*|[ugo]))+|[-+=][0-7]+'.\n");
    }
    else
        fprintf(stderr, "Try '%s --help' for more information.\n", PROGRAM_NAME);
    exit(status);
}

static void
mode_error()
{
    fprintf(stderr, "%s: invalid mode: '%s'\n", PROGRAM_NAME, mode_list);
    usage(EXIT_FAILURE);
}

static int
string2octal(const char *str, const char *end)
{
    int sum = 0;
    while (str != end && *str && ISOCTAL(*str))
    {
        sum = sum * 8 + (*str - '0');
        ++str;
    }

    if (str != end && *str)
        mode_error();

    return sum;
}

static char *
mode2str(int mode)
{
    static char buf[10];

    strncpy(buf, "---------", 10);

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (mode & modes[i][j])
                buf[i * 3 + j] = mc[j];

    if (mode & S_ISUID)
        buf[2] = (buf[2] == 'x') ? 's' : 'S';
    if (mode & S_ISGID)
        buf[5] = (buf[5] == 'x') ? 's' : 'S';

    return buf;
}

static void
print_info(const char *filename, mode_t old_mode, mode_t new_mode)
{
    if (statbuf.st_mode != old_mode)
    {
        if (cflag || vflag)
            printf("mode of '%s' changed from %04o (%s) to %04o (%s)\n", filename,
                   GET_MODE_BIT(old_mode), mode2str(old_mode), GET_MODE_BIT(new_mode), mode2str(new_mode));
    }
    else
    {
        if (vflag)
            printf("mode of '%s' retained as %04o (%s)\n", filename,
                   GET_MODE_BIT(old_mode), mode2str(old_mode));
    }
}

static bool
do_recursive(const char *cur, bool (*func)(const char *))
{
    char *child_file = (char *)malloc(BUFSIZ * sizeof(char));
    DIR *dir;
    struct dirent *entry;
    bool ok = true;

    if ((dir = opendir(cur)) == NULL)
    {
        fprintf(stderr, "%s: failed to open directory %s: %s\n",
                PROGRAM_NAME, cur, strerror(errno));
        return false;
    }

    while ((entry = readdir(dir)))
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            // 构造文件名
            if (cur[strlen(cur) - 1] == '/')
                sprintf(child_file, "%s%s", cur, entry->d_name);
            else
                sprintf(child_file, "%s/%s", cur, entry->d_name);
            // 执行回调函数
            ok &= func(child_file);
        }
    }

    if (errno)
    {
        fprintf(stderr, "%s: cannot read directory '%s': %s\n",
                PROGRAM_NAME, cur, strerror(errno));
        return false;
    }
    if (closedir(dir))
    {
        fprintf(stderr, "%s: failed to close directory %s: %s\n",
                PROGRAM_NAME, cur, strerror(errno));
        return false;
    }

    free(child_file);

    return ok;
}

static bool
process_mode_list(const char *filename)
{
    bool ok = true;

    mode_t old_mode;

    // 1. 获取文件最初的mode
    if (stat(filename, &statbuf) < 0)
    {
        fprintf(stderr, "%s: cannot access '%s': %s\n", PROGRAM_NAME, filename, strerror(errno));
        return false;
    }
    old_mode = statbuf.st_mode;

    bool flags[3] = {0, 0, 0};
    int old_bits[3] = {0, 0, 0};

    // 2. 获取初始mode bit
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 4; ++j)
            if (statbuf.st_mode & modes[i][j])
                old_bits[i] |= 1 << j;
    }

    // 清空现有mode bit
    statbuf.st_mode = CLEAR_MODE_BIT(statbuf.st_mode);

    // 3. parse mode[,mode]...
    const char *start, *end, *op;

    start = mode_list;
    while (true)
    {
        int idx; // 用来记录op后面的[ugo]对应的索引

        // 1. 找到op和",或者mode_list结尾"
        end = start;
        op = NULL;
        while (*end && *end != ',')
        {
            if (*end == '=' || *end == '+' || *end == '-')
                op = end;
            ++end;
        }

        // 如果没有op
        if (!op)
            mode_error();

        // 如果至少有一个操作符且操作符后至少有一个字符，即不是"+" "-" "="
        if (ISOCTAL(*(op + 1)))
        {
            if (start != op)
                mode_error();
            int m = string2octal(op + 1, end);

            switch (*op)
            {
            case '+':
                for (int i = 0; i < 3; ++i)
                    for (int j = 0; j < 4; ++j)
                        if (m & modes[i][j])
                            old_bits[i] |= 1 << j;
                break;
            case '-':
                for (int i = 0; i < 3; ++i)
                    for (int j = 0; j < 4; ++j)
                        if (m & modes[i][j])
                            old_bits[i] &= ~(1 << j);
                break;
            case '=':
                for (int i = 0; i < 3; ++i)
                {
                    old_bits[i] = 0;
                    for (int j = 0; j < 4; ++j)
                        if (m & modes[i][j])
                        {
                            old_bits[i] |= 1 << j;
                        }
                }

                break;
            default:
                mode_error();
            }
        }
        else
        {
            // 对于op前面的组别
            int all_flag = search(start, op, 'a') || op == start; // 默认不指定是a

            // 重置flags
            for (int i = 0; i < 3; ++i)
            {
                if (search(start, op, who[i]) || all_flag)
                    flags[i] = true;
                else
                    flags[i] = false;
            }

            // 构造对应的mode
            mode_t m;
            char *op2 = op;

            while (op2 < end)
            {
                m = 0;
                char c = *op2;

                if ((idx = get_idx(who, 3, *(op2 + 1))) != -1)
                {
                    if (op2 + 2 < end)
                        mode_error();

                    // 获取该用户的mode
                    m = old_bits[idx];
                    op2 += 2;
                }
                else if (ISRWX(*(op + 1)))
                {
                    ++op2;
                    int idx;
                    while (op2 != end && !ISOP(*op2))
                    {
                        if ((idx = get_idx(mc, 4, *op2)) == -1)
                            mode_error();
                        m |= 1 << idx;
                        ++op2;
                    }
                }
                else
                    ++op2;

                for (int i = 0; i < 3; ++i)
                {
                    if (flags[i])
                    {
                        // 清空所有标志
                        if (c == '=')
                            old_bits[i] = m;
                        else if (c == '-')
                            old_bits[i] &= ~m;
                        else if (c == '+')
                            old_bits[i] |= m;
                        else
                            mode_error();
                    }
                }
            }
        }

        // 判断是否处理完了所有mode
        if (*end)
            start = end + 1;
        else
            break;
    }

    // reset mode bit
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 4; ++j)
            if (old_bits[i] & (1 << j))
                statbuf.st_mode |= modes[i][j];
    }

    chmod(filename, statbuf.st_mode);
    print_info(filename, old_mode, statbuf.st_mode);

    // 如果是目录且Rflag
    if (S_ISDIR(statbuf.st_mode) && Rflag)
        ok &= do_recursive(filename, process_mode_list);

    return ok;
}

static bool
process_mode(const char *filename)
{
    bool ok = true;

    mode_t old_mode;

    if (stat(filename, &statbuf) < 0)
    {
        fprintf("%s: failed to get attributes of '%s': %s\n",
                PROGRAM_NAME, rfile, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // clear all bit
    old_mode = GET_MODE_BIT(statbuf.st_mode);
    statbuf.st_mode = CLEAR_MODE_BIT(statbuf.st_mode);
    statbuf.st_mode |= mode;

    ok = chmod(filename, statbuf.st_mode) == 0;

    if (!ok)
    {
        fprintf(stderr, "%s: changing permissions of '%s': %s\n",
                PROGRAM_NAME, filename, strerror(errno));
        fprintf(stderr, "failed to change mode of '%s' from %d (%s) to %d (%s)\n",
                filename, old_mode, mode2str(old_mode), statbuf.st_mode, mode2str(statbuf.st_mode));
    }
    else
        print_info(filename, old_mode, statbuf.st_mode);

    // 如果是目录且Rflag
    if (S_ISDIR(statbuf.st_mode) && Rflag)
        ok &= do_recursive(filename, process_mode);

    return ok;
}

int main(int argc, char **argv)
{
    bool ok = true;

    /* Parse argument */
    int o;
    while ((o = getopt_long(argc, argv, "cfvRr::w::x::s::u::g::o::a::+::=::"
                                        "0::1::2::3::4::5::6::7::",
                            long_options, NULL)) != -1)
    {
        /* code */
        switch (o)
        {
        case 'r':
        case 'w':
        case 'x':
        case 's':
        case '0' ... '7':
        {
            const char *arg = argv[optind - 1];
            int arg_len = strlen(arg);
            int mode_comma_len = mode_len + !!mode_len;
            int new_mode_len = mode_comma_len + arg_len;

            if (mode_alloc <= new_mode_len)
            {
                mode_alloc = new_mode_len + 1;
                mode_list = realloc(mode, mode_alloc);
            }

            mode_list[mode_len] = ',';
            strncpy(mode_list + mode_comma_len, arg, arg_len);
            mode_len = new_mode_len;
        }
            break;
        case 'c':
            cflag = true;
            break;
        case 'f':
            break;
        case 'v':
            vflag = true;
            break;
        case 'R':
            Rflag = true;
            break;

            case_GETOPT_HELP_CHAR;
            case_GETOPT_VERSION_CHAR;

        case NO_PRESERVE_ROOT:
            break;

        case PRESERVE_ROOT:
            printf("Written by rda\n");
            break;

        case REFERENCE:
            rfile = optarg;
            break;

        default:
            usage(EXIT_FAILURE);
        }
    }

    /* Parse mode */
    if (rfile)
    {
        // 检查是否接收到了mode_list，即"-r" "-rw"类型的，被get_opt认为是mode_list的sth
        if (mode_len)
        {
            fprintf(stderr, "%s: cannot combine mode and --reference options\n");
            usage(EXIT_FAILURE);
        }
        if (stat(rfile, &statbuf) < 0)
        {
            fprintf(stderr, "%s: failed to get attributes of '%s': %s\n",
                    PROGRAM_NAME, rfile, strerror(errno));
            exit(EXIT_FAILURE);
        }
        mode = GET_MODE_BIT(statbuf.st_mode);
        printf("rfile: %s %o\n", rfile, mode);

        has_parse_mode = true;
    }
    else
    {
        if (!mode_len)
            mode_list = argv[optind++];
        // OCTAL-MODE
        if (isdigit(mode_list[0]))
        {
            mode = string2octal(mode_list, NULL);
            mode = GET_MODE_BIT(mode);
            has_parse_mode = true;
        }
        // else
        // MODE,[MODE]...
    }

    if (has_parse_mode)
    {
        if (mode > 07777)
            mode_error();
    }

    if (optind == argc)
    {
        fprintf(stderr, "%s: missing operand after '%s'\n", PROGRAM_NAME, argv[optind - 1]);
        usage(EXIT_FAILURE);
    }

    /* chmod file */
    char **filelist = &argv[optind];

    if (has_parse_mode)
    {
        while (*filelist)
            ok &= process_mode(*filelist++);
    }
    else
    {
        while (*filelist)
            ok &= process_mode_list(*filelist++);
    }

    if (mode_len) // 分配了空间存放mode_list
        free(mode_list);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
