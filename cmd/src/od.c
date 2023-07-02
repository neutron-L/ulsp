#include <assert.h>
#include <locale.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include "apue.h"
#include "system.h"

#define PROGRAM_NAME "od"
#define DEFAULT_NUMBER 10

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static int Nflag;
static int width = 16, bytes_to_read, skip_bytes; 
static int bytes_has_read;
static int line_num_start;
static int base = 8;

static char buf[BUFSIZ];
static char * bpin = buf;
static size_t len = 0; // 当前buf中的字节数

static struct option const long_options[] =
    {
        {"address-radix", required_argument, NULL, 'A'},
        {"skip-bytes", required_argument, NULL, 'j'},
        {"read-bytes", required_argument, NULL, 'N'},
        {"zero-strings", no_argument, NULL, 'S'},
        {"format", required_argument, NULL, 't'},
        {"output-duplicates", no_argument, NULL, 'v'},
        {"width", optional_argument, NULL, 'w'},
        {GETOPT_HELP_OPTION_DECL},
        {GETOPT_VERSION_OPTION_DECL},
        {NULL, 0, NULL, 0}};


void
usage(int status)
{
    if (status == EXIT_SUCCESS)
    {
        printf("Usage: od [OPTION]... [FILE]...\n");
        printf("  or:  od [-abcdfilosx]... [FILE] [[+]OFFSET[.][b]]\n");
        printf("  or:  od --traditional [OPTION]... [FILE] [[+]OFFSET[.][b] [+][LABEL][.][b]]\n");
        printf("\nWrite an unambiguous representation, octal bytes by default, \
            of FILE to standard output.  With more than one FILE argument, \
            concatenate them in the listed order to form the input.\n");
        printf("\nWith no FILE, or when FILE is -, read standard input.\n");
        printf("\nIf first and second call formats both apply, the second format is assumed\n");

    }
    else
        fprintf(stderr, "Try '%s --help' for more information.\n", PROGRAM_NAME);
    exit(status);
}

static bool
od(const char * filename)
{
    int fd;

    // open file
    if ((fd = open(filename, O_RDONLY)) < 0)
    {
        fprintf(stderr, "%s: %d: read file %s error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }

    // 还有需要跳过的字节
    if (skip_bytes)
    {
        ssize_t filesize = lseek(fd, 0, SEEK_END);
        if (skip_bytes >= filesize) // 不需要读次文件了 直接返回
        {
            skip_bytes -= filesize;
            close(fd);
            
            return true;
        }
        else
        {
            // 定位文件
            lseek(fd, skip_bytes, SEEK_SET);
            skip_bytes = 0;
        }
    }

    
    ssize_t bytes_read = 0;

    // read file
    ssize_t i = 0;
    while ((!Nflag || bytes_to_read))
    {
        // Note: 不可能读出多余的字节存入buf
        bytes_read = buf + BUFSIZ - bpin;
        if (Nflag)
            bytes_read = MIN(bytes_read, bytes_to_read);
        bytes_read = read(fd, &buf[len], bytes_read);
        

        if (bytes_read == 0)
            break;
        else if (bytes_read < 0)
        {
            fprintf(stderr, "%s: %d: read file %s error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
            close(fd);
            return false;
        }

        if (Nflag)
            bytes_to_read -= bytes_read;
        len += bytes_read;

        // scan buf
        for (i = 0; i < len - 1; i += 2)
        {
            // 输出行号
            if (bytes_has_read % width == 0)
            {
                if (bytes_has_read)
                    printf("\n");
                printf("%07o", bytes_has_read + line_num_start);
            }

            // 按两个字节打印八进制表示
            printf(" %06o", *(short *)&buf[i]);

            bytes_has_read += 2;
        }

        // 多余的一个字节移动到最前面
        if (i == len - 1)
        {
            memcpy(buf, &buf[i], 1);
            len = 1;
        }
        else
            len = 0;
    }

    // close file
    if (close(fd) < 0)
    {
        fprintf(stderr, "%s: %d: close file %s error: %s\n",
                    __FUNCTION__, __LINE__, filename, strerror(errno));
        return false;
    }

    return true;
}

int main(int argc, char ** argv)
{
    bool ok = true;

    int o;
    /* parse arguments */
    while ((o = getopt_long(argc, argv, "Aj:N:S:t:vw::", long_options, NULL)) != -1)
    {
        switch (o)
        {
        case 'A':
            /* Do nothing */
            break;
        case 'j':
            skip_bytes = atoi(optarg);
            break;
        case 'N':
            Nflag = 1;
            bytes_to_read = atoi(optarg);
            break;
        case 'S':
            break;
        case 't':
            break;
        case 'v':
            break;
        case 'w':
            width = atoi(optarg);
            break;
        case_GETOPT_HELP_CHAR;
        case_GETOPT_VERSION_CHAR;        
        default:
            usage(EXIT_FAILURE);
        }
    }
    
    line_num_start = skip_bytes;
    /* Main loop */
    for (int i = optind; i < argc; ++i)
        ok &= od(argv[i]);

    assert(bytes_to_read == 0);
    // 打印最后的行号
    if (len) // 还有多余的一个字节没打印
    {
        if (bytes_has_read % width == 0)
        {
            if (bytes_has_read)
                printf("\n");
            printf("%07o", bytes_has_read + line_num_start);
        }

        // 打印多余的一个字节
        printf(" %06o", buf[0]);
        ++bytes_has_read;
    }
    // 打印最后的行号
    printf("\n%07o\n", bytes_has_read + line_num_start);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
