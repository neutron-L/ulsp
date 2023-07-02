#include <locale.h>
#include <stdbool.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "apue.h"

#define PROGRAM_NAME "head"
#define DEFAULT_NUMBER 10

enum
{
    GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    GETOPT_VERSION_CHAR = (CHAR_MIN - 3)
};

#define GETOPT_HELP_OPTION_DECL \
    "help", no_argument, NULL, GETOPT_HELP_CHAR
#define GETOPT_VERSION_OPTION_DECL \
    "version", no_argument, NULL, GETOPT_VERSION_CHAR

#define case_GETOPT_HELP_CHAR \
    case GETOPT_HELP_CHAR:    \
        usage(EXIT_SUCCESS);  \
        break

#define case_GETOPT_VERSION_CHAR    \
    case GETOPT_VERSION_CHAR:       \
        printf("Written by rda\n"); \
        break

static int cflag, nflag = 1, qflag, vflag;
static int elide_from_end;       // 是否打印除了文件末尾num个字符或者行的其余内容
static int num = DEFAULT_NUMBER; // -c或-n指定的数字（绝对值）
static int print_headers;        // 是否需要打印文件名
static char line_end = '\n';

static struct option const long_options[] =
    {
        {"bytes", required_argument, NULL, 'c'},
        {"lines", required_argument, NULL, 'n'},
        {"quiet", no_argument, NULL, 'q'},
        {"verbose", no_argument, NULL, 'v'},
        {"zero-terminated", no_argument, NULL, 'z'},
        {GETOPT_HELP_OPTION_DECL},
        {GETOPT_VERSION_OPTION_DECL},
        {NULL, 0, NULL, 0}};

void usage(int status)
{
    if (status == EXIT_SUCCESS)
    {
        printf("Usage: head [OPTION]... [FILE]...\n");
        printf("Print the first 10 lines of each FILE to standard output.\n");
        printf("With more than one FILE, precede each with a header giving the file name.\n");

        printf("\nWith no FILE, or when FILE is -, read standard input.\n");
        printf("\nMandatory arguments to long options are mandatory for short options too.\n");
        printf("  -c, --bytes=[-]NUM       print the first NUM bytes of each file;\n");
        printf("                            with the leading '-', print all but the last\n");
        printf("                             NUM bytes of each file\n");
        printf("  -n, --lines=[-]NUM       print the first NUM lines instead of the first 10;\n");
        printf("                             with the leading '-', print all but the last\n");
        printf("                             NUM lines of each file\n");
        printf("  -q, --quiet, --silent    never print headers giving file names\n");
        printf("  -v, --verbose            always print headers giving file names\n");
        printf("  -z, --zero-terminated    line delimiter is NUL, not newline\n");
        printf("      --help     display this help and exit\n");
        printf("      --version  output version information and exit\n");
    }
    else
        fprintf(stderr, "Try 'head --help' for more information.\n");
    exit(status);
}

static void
write_header(char const *filename)
{
    static bool first_line = true;

    printf("%s==> %s <==\n", (first_line ? "" : "\n"), filename);
    fflush(stdout); // 其他地方是用write写入标准输出，当标准输出被重定向到文件时，此处的输出可能不会立刻写入文件，因此需要flush

    first_line = false;
}

static void
xwrite_stdout(char const *buffer, size_t n_bytes)
{
    if (n_bytes > 0 && fwrite(buffer, 1, n_bytes, stdout) < n_bytes)
    {
        clearerr(stdout);
        fprintf(stderr, "write error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static size_t
string_to_integer(const char *const str)
{
    size_t res = 0;
    const char *p = str;

    while (*p)
    {
        res = res * 10 + (*p - '0');
        ++p;
    }

    return res;
}

static bool
copy_fd(int fd, size_t n_bytes)
{
    char buf[BUFSIZ];
    size_t bytes_to_read = BUFSIZ;

    while (n_bytes > 0)
    {
        bytes_to_read = bytes_to_read > n_bytes ? n_bytes : bytes_to_read;
        ssize_t bytes_read = read(fd, buf, bytes_to_read);
        if (bytes_read < 0)
        {
            fprintf(stderr, "%s: %d: read file error: %s\n",
                    __FUNCTION__, __LINE__, strerror(errno));
            return false;
        }
        else if (bytes_read == 0)
            break;
        xwrite_stdout(buf, bytes_read);
        n_bytes -= bytes_read;
    }

    return true;
}

static bool
head_lines(char const *filename, int fd)
{
    char buf[BUFSIZ];
    int lines_to_write = num;

    while (lines_to_write)
    {
        ssize_t bytes_read = read(fd, buf, BUFSIZ);
        size_t bytes_to_write = 0;

        if (bytes_read < 0)
        {
            perror("read file error: ");
            exit(EXIT_FAILURE);
        }
        else if (bytes_read == 0)
            break;

        while (bytes_to_write < bytes_read)
        {
            if (buf[bytes_to_write++] == line_end && --lines_to_write == 0)
            {
                if (lseek(fd, bytes_to_write - bytes_read, SEEK_CUR) < 0)
                {
                    fprintf(stderr, "%s: %d: file %s seek error: %s\n",
                            __FUNCTION__, __LINE__, filename, strerror(errno));
                    return false;
                }
                break;
            }
        }
        xwrite_stdout(buf, bytes_to_write);
    }

    return true;
}

static bool
head_bytes(char const *filename, int fd)
{
    return copy_fd(fd, num);
}

static bool
elide_tail_lines_file(char const *filename, int fd)
{
    char buf[BUFSIZ];
    int n_lines = num;
    size_t size;
    ssize_t bytes_read;
    int pos;
    int n = 0;

    struct stat stat_buf;
    if (fstat(fd, &stat_buf) < 0)
    {
        fprintf(stderr, "%s: %d: file %s stat error: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }
    size = stat_buf.st_size;
    pos = size;

    if (!n_lines || !size)
        return copy_fd(fd, size);
    bytes_read = size % BUFSIZ;
    pos -= bytes_read;
    lseek(fd, pos, SEEK_SET);
    bytes_read = read(fd, buf, bytes_read);

    if (bytes_read < 0)
    {
        fprintf(stderr, "%s: %d: file %s read error: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }

    if (bytes_read && buf[bytes_read - 1] != '\n')
        --n_lines;

    while (true)
    {
        n = bytes_read;

        // 处理当前缓冲区中的'\n'
        while (n)
        {
            const char *ptr;
            ptr = memrchr(buf, line_end, n);
            if (ptr == NULL)
                break;
            n = ptr - buf;

            // 扫描完倒数的n_lines行
            if (n_lines-- == 0)
            {
                if (lseek(fd, 0, SEEK_SET) < 0)
                {
                    fprintf(stderr, "%s: %d: file %s seek error: %s\n",
                            __FUNCTION__, __LINE__, filename, strerror(errno));
                    return false;
                }
                if (!copy_fd(fd, pos))
                {
                    return false;
                }
                xwrite_stdout(buf, n + 1); // +1是输出最后一个'\n'
                return true;
            }
        }

        if (pos == 0)
            return true;

        pos -= BUFSIZ;
        if (lseek(fd, pos, SEEK_SET) < 0)
        {
            fprintf(stderr, "%s: %d: file %s seek error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
            return false;
        }
        bytes_read = read(fd, buf, BUFSIZ);

        if (bytes_read < 0)
        {
            fprintf(stderr, "%s: %d: file %s read error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
            return false;
        }
        else if (bytes_read == 0)
            break;
    }

    return true;
}

static bool
elide_tail_bytes_file(char const *filename, int fd)
{
    bool ok = true;

    size_t num_copy = num;
    size_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    num = size - num;
    if (num > 0)
        ok = head_bytes(filename, fd);
    num = num_copy;

    return ok;
}

static bool
head(char const *filename, int fd)
{
    if (print_headers)
        write_header(filename);

    if (elide_from_end)
    {
        if (nflag)
            return elide_tail_lines_file(filename, fd);
        else
            return elide_tail_bytes_file(filename, fd);
    }

    if (nflag)
        return head_lines(filename, fd);
    else
        return head_bytes(filename, fd);
}

static bool
head_file(char const *filename)
{
    bool ok = true;
    int fd;

    if (!strcmp(filename, "-"))
    {
        fd = STDIN_FILENO;
        filename = "standard input";
    }
    else
    {
        fd = open(filename, O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "head: cannot open \'%s\' for reading: %s\n", filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    ok = head(filename, fd);
    if (close(fd) < 0)
    {
        perror("close file error: ");
        exit(EXIT_FAILURE);
    }

    return ok;
}

int main(int argc, char **argv)
{
    bool ok = true;
    static const char *const default_file_list[] = {"-", NULL};
    char const *const *file_list;

    int c;
    while ((c = getopt_long(argc, argv, "c:n:qvz", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'c':
            cflag = 1;
            nflag = 0;
            elide_from_end = (*optarg == '-');
            if (elide_from_end)
                ++optarg;
            num = string_to_integer(optarg);
            break;
        case 'n':
            cflag = 0;
            nflag = 1;
            elide_from_end = (*optarg == '-');
            if (elide_from_end)
                ++optarg;
            num = string_to_integer(optarg);
            break;
        case 'q':
            qflag = 1;
            break;
        case 'v':
            vflag = 1;
            break;
        case 'z':
            line_end = '\0';
            break;
            case_GETOPT_HELP_CHAR;
            case_GETOPT_VERSION_CHAR;
        default:
            usage(EXIT_FAILURE);
        }
    }

    if (vflag || (!qflag && optind < argc - 1))
        print_headers = 1;

    file_list = optind < argc ? (char const *const *)&argv[optind] : default_file_list;

    for (int i = 0; file_list[i]; ++i)
        ok &= head_file(file_list[i]);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}