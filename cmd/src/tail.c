#include <assert.h>
#include <locale.h>
#include <stdbool.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "apue.h"

#define PROGRAM_NAME "tail"
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

/* options flag */
static int cflag, nflag, fflag, qflag, sflag, vflag;
static char line_end = '\n';
static int num;
static int elide_from_begin;
static int print_header;

static struct option const long_options[] =
    {
        {"bytes", required_argument, NULL, 'c'},
        {"follow", required_argument, NULL, 'f'},
        {"lines", required_argument, NULL, 'n'},
        {"quiet", no_argument, NULL, 'q'},
        {"silent", no_argument, NULL, 'q'},
        {"verbose", no_argument, NULL, 'v'},
        {"zero-terminated", no_argument, NULL, 'z'},
        {GETOPT_HELP_OPTION_DECL},
        {GETOPT_VERSION_OPTION_DECL},
        {NULL, 0, NULL, 0}};

void usage(int status)
{
    if (status == EXIT_SUCCESS)
    {
        printf("Usage: tail [OPTION]... [FILE]...\n");
        printf("Print the last 10 lines of each FILE to standard output.\n");
        printf("With more than one FILE, precede each with a header giving the file name.\n");
        printf("\nWith no FILE, or when FILE is -, read standard input.\n");
        printf("\nMandatory arguments to long options are mandatory for short options too.\n");
        printf("  -c, --bytes=[+]NUM       output the last NUM bytes; or use -c +NUM to\n");
        printf("                             output starting with byte NUM of each file\n");
        printf("  -f, --follow[={name|descriptor}]\n");
        printf("  -n, --lines=[+]NUM       output the last NUM lines, instead of the last 10;\n");
        printf("                             or use -n +NUM to output starting with line NUM\n");
        printf("  -q, --quiet, --silent    never output headers giving file names\n");
        printf("  -v, --verbose            always output headers giving file names\n");
        printf("  -z, --zero-terminated    line delimiter is NUL, not newline\n");
        printf("      --help     display this help and exit\n");
        printf("      --version  output version information and exit\n");
    }
    else
        fprintf(stderr, "Try 'tail --help' for more information.\n");
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
tail_lines(char const *filename, int fd, int filesize)
{
    bool ok = true;
    char buf[BUFSIZ];
    int pos;
    ssize_t bytes_read;
    size_t n_lines = num;

    // 如果文件为空直接返回ok
    if (!filesize || !n_lines)
        return ok;

    pos = filesize;
    bytes_read = pos % BUFSIZ;
    if (bytes_read == 0)
        bytes_read = BUFSIZ;
    pos -= bytes_read;

    if (lseek(fd, pos, SEEK_SET) < 0)
    {
        fprintf(stderr, "%s: %d: seek file %s error: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }
    
    if ((bytes_read = read(fd, buf, bytes_read)) < 0)
    {
        fprintf(stderr, "%s: %d: read file %s error: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }

    if (buf[bytes_read - 1] != '\n')
        --n_lines;
    
    while (true)
    {
        size_t n = bytes_read;
        while (n)
        {
            const char *ptr;
            // ptr = memrchr(buf, line_end, n);
            ptr = buf + n - 1;
            while (ptr >= buf && *ptr != line_end)
                --ptr;
            if (ptr < buf)
                break;
            n = ptr - buf;

            // backwards scan n_lines行完成
            if (n_lines-- == 0)
            {
                pos += n + 1;
                if (lseek(fd, pos, SEEK_SET) < 0)
                {
                    fprintf(stderr, "%s: %d: seek file %s error: %s\n",
                            __FUNCTION__, __LINE__, filename, strerror(errno));
                    return false;
                }
                return copy_fd(fd, filesize - pos);
            }
        }
        if (pos == 0)
        {
            if (lseek(fd, pos, SEEK_SET) < 0)
            {
                fprintf(stderr, "%s: %d: seek file %s error: %s\n",
                        __FUNCTION__, __LINE__, filename, strerror(errno));
                return false;
            }
            return copy_fd(fd, filesize - pos);
        }
        pos -= BUFSIZ;
        if (lseek(fd, pos, SEEK_SET) < 0)
        {
            fprintf(stderr, "%s: %d: seek file %s error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
            return false;
        }
        if ((bytes_read = read(fd, buf, BUFSIZ)) < 0)
        {
            fprintf(stderr, "%s: %d: read file %s error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
            return false;
        }
        assert(bytes_read > 0);
    }

    return ok;
}

static bool
tail_bytes(char const *filename, int fd, size_t filesize)
{
    int pos = filesize - num;

    if (pos < 0)
        pos = 0;
    if (lseek(fd, pos, SEEK_SET) < 0)
    {
        fprintf(stderr, "%s: %d: fstat file %s error: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }

    return copy_fd(fd, num);
}

static bool
elide_head_lines(char const *filename, int fd, size_t filesize)
{
    bool ok = true;
    char buf[BUFSIZ];
    int pos;
    ssize_t bytes_read;
    size_t n_lines = num;

    // 如果文件为空直接返回ok
    if (!filesize || !n_lines)
        return ok;
    --n_lines;
    pos = 0;
    bytes_read = BUFSIZ;

    if (lseek(fd, pos, SEEK_SET) < 0)
    {
        fprintf(stderr, "%s: %d: seek file %s error: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }

    if (!n_lines)
        return copy_fd(fd, filesize);
    
    if ((bytes_read = read(fd, buf, bytes_read)) < 0)
    {
        fprintf(stderr, "%s: %d: read file %s error: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }

    while (true)
    {
        size_t n = bytes_read;
        const char *ptr = buf;
        size_t step;

        while (n)
        {
            ptr = memchr(ptr, line_end, n);
            if (ptr == NULL)
                break;
            ++ptr;
            step = ptr - buf;
            n -= step;
            // backwards scan n_lines行完成
            if (--n_lines == 0)
            {
                pos += step;
                if (lseek(fd, pos, SEEK_SET) < 0)
                {
                    fprintf(stderr, "%s: %d: seek file %s error: %s\n",
                            __FUNCTION__, __LINE__, filename, strerror(errno));
                    return false;
                }
                return copy_fd(fd, filesize - pos);
            }
        }
        
        pos += bytes_read;
        if (pos > filesize)
            pos = filesize;
        if (pos == filesize)
            break;

        if (lseek(fd, pos, SEEK_SET) < 0)
        {
            fprintf(stderr, "%s: %d: seek file %s error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
            return false;
        }
        if ((bytes_read = read(fd, buf, BUFSIZ)) < 0)
        {
            fprintf(stderr, "%s: %d: read file %s error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
            return false;
        }
        assert(bytes_read > 0);
    }

    return ok;
}

static bool
elide_head_bytes(char const *filename, int fd, size_t filesize)
{
    bool ok = true;
    size_t num_copy = num;
    num = filesize - num + 1;
    ok = tail_bytes(filename, fd, filesize);
    num = num_copy;

    return ok;
}

static bool
tail(char const *filename, int fd)
{
    struct stat stat_buf;
    size_t size;

    if (fstat(fd, &stat_buf) < 0)
    {
        fprintf(stderr, "%s: %d: fstat file %s error: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }
    size = stat_buf.st_size;

    if (print_header)
        write_header(filename);

    if (elide_from_begin)
    {
        if (cflag)
            return elide_head_bytes(filename, fd, size);
        else
            return elide_head_lines(filename, fd, size);
    }
    if (cflag)
        return tail_bytes(filename, fd, size);
    else
        return tail_lines(filename, fd, size);
}

static bool
tail_file(char const *filename)
{
    int fd;
    bool ok;

    if ((fd = open(filename, O_RDONLY)) < 0)
    {
        fprintf(stderr, "%s: %d: cannot open file %s: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }

    ok = tail(filename, fd);

    if (close(fd) < 0)
    {
        fprintf(stderr, "%s: %d: cannot close file %s: %s\n",
                __FUNCTION__, __LINE__, filename, strerror(errno));
        ok = false;
    }

    return ok;
}

int main(int argc, char **argv)
{
    bool ok = true;
    const char *const default_file_list[] = {"-", NULL};
    const char *const *file_list;

    /* Parse argumetns */
    int c;
    while ((c = getopt_long(argc, argv, "c:f:n:Fqsvz", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'c':
            cflag = 1;
            nflag = 0;
            elide_from_begin = optarg[0] == '+';
            if (elide_from_begin)
                ++optarg;
            num = string_to_integer(optarg);

            break;
        case 'f':
            fflag = 1;
            break;
        case 'n':
            nflag = 1;
            cflag = 0;

            elide_from_begin = optarg[0] == '+';
            if (elide_from_begin)
                ++optarg;
            num = string_to_integer(optarg);

            break;
        case 'q':
            qflag = 1;
            break;
        case 's':
            sflag = 1;
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
        }
    }
    file_list = optind < argc ? (const char *const *)&argv[optind] : default_file_list;

    if (vflag || (!qflag && optind < argc - 1))
        print_header = 1;
    /* Main loop */
    int i = 0;
    while (file_list[i])
        ok &= tail_file(file_list[i++]);
    // printf("%s\n", file_list[i++]);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}