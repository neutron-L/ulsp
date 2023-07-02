#include <locale.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>

#include "apue.h"

#define PROGRAM_NAME "cat"

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
        exit(EXIT_SUCCESS)


/* Name of input file. May be "-". */
static char const *infile;

/* Descriptor on which input file is open */
static int input_desc;

/* Buffer for line numbers. */
#define LINE_COUNTER_BUF_LEN 20
static char line_buf[LINE_COUNTER_BUF_LEN] =
    {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '0',
        '\t', '\0'};

/* Position in 'line_buf' where printing starts. This whill not change ... */
static char *line_num_print = line_buf + LINE_COUNTER_BUF_LEN - 8;
/* Position of the first digit in 'line_buf'. */
static char *line_num_start = line_buf + LINE_COUNTER_BUF_LEN - 3;
/* Position of the last digit in 'line_buf'. */
static char *line_num_end = line_buf + LINE_COUNTER_BUF_LEN - 3;

/* Preserves the 'cat' function's local 'newlines' between invocations.  */
static int newlines2 = 0;

/* options flags */
static int Aflag, bflag, eflag, Eflag, nflag, sflag, tflag, Tflag, uflag, vflag;

void usage(int status)
{
    if (status == EXIT_SUCCESS)
    {
        printf("Usage: cat [OPTION]... [FILE]...\n");
        printf("Concatenate FILE(s) to standard output.\n\n");
        printf("With no FILE, or when FILE is -, read standard input.\n\n");
        printf("  -A, --show-all           equivalent to -vET\n");
        printf("  -b, --number-nonblank    number nonempty output lines, overrides -n\n");
        printf("  -e                       equivalent to -vE\n");
        printf("  -E, --show-ends          display $ at end of each line\n");
        printf("  -n, --number             number all output lines\n");
        printf("  -t                       equivalent to -vT\n");
        printf("  -s, --squeeze-blank      suppress repeated empty output lines\n");
        printf("  -T, --show-tabs          display TAB characters as ^I\n");
        printf("  -u                       (ignored)\n");
        printf("  -v, --show-nonprinting   use ^ and M- notation, except for LFD and TAB\n");
        printf("      --help     display this help and exit\n");
        printf("      --version  output version information and exit\n");
        printf("\nExamples:\n"
               "  cat f - g  Output f's contents, then standard input, then g's contents.\n"
               "  cat        Copy standard input to standard output.\n");
    }
    exit(status);
}

/* Compute the next line number */
static void
next_line_num()
{
    char *endp = line_num_end;

    do
    {
        if ((*endp)++ < '9')
            return;
        *endp-- = '0';
    } while (endp >= line_num_start);

    if (line_num_start > line_buf)
        *--line_num_start = '1';
    else
        *line_buf = '>';
    if (line_num_start < line_num_print)
        --line_num_print;
}

static ssize_t
safe_read(int fd, char *buf, size_t size)
{
    ssize_t n;
    char *ptr = buf;
    size_t rem = size;

    while (rem && (n = read(fd, ptr, rem)) >= 0)
    {
        if (n < 0)
        {
            perror("read error: ");
            return n;
        }
        else if (errno == EINTR)
            continue;
        else if (n == 0)
            break;
        ptr += n;
        rem -= n;
    }

    return size - rem;
}

static ssize_t
safe_write(int fd, char *buf, size_t size)
{
    ssize_t n;
    char *ptr = buf;
    size_t rem = size;

    while (rem && (n = write(fd, ptr, rem)) >= 0)
    {
        if (n < 0)
        {
            perror("read error: ");
            return -1;
        }
        else if (errno == EINTR)
            continue;
        ptr += n;
        rem -= n;
    }

    return size - rem;
}

/* Write any pending output to STDOUT_FILENO.
   Pending is defined to be the *BPOUT - OUTBUF bytes starting at OUTBUF.
   Then set *BPOUT to OUTPUT if it's not already that value.  */

static inline void
write_pending(char *outbuf, char **bpout)
{
    size_t n_write = *bpout - outbuf;
    if (0 < n_write)
    {
        if (safe_write(STDOUT_FILENO, outbuf, n_write) != n_write)
            perror("write error");
        *bpout = outbuf;
    }
}

static void
simple_cat(char *buf, size_t bufsize)
{
}

static bool
cat(char *inbuf, size_t insize, char *outbuf, size_t outsize)
{
    /* Last character read from the input buf */
    unsigned char ch;

    int newlines = newlines2;

    /* Pointer to the first non-valid byte in the input buffer */
    char *eob = inbuf;
    /* Pointer to the next character in the input buffer. */
    char *bpin = eob + 1;

    /* Pointer to the position where the next character shall be written */
    char *bpout = outbuf;

    while (true)
    {
        do
        {
            // flush output buffer
            if (outbuf + outsize <= bpout)
            {
                char *wout = outbuf;
                size_t rem = bpout - outbuf;
                do
                {
                    /* code */
                    if (safe_write(STDOUT_FILENO, wout, outsize) != outsize)
                    {
                        // error
                        fprintf(stderr, "safe write failed\n");
                        exit(1);
                    }
                    wout += outsize;
                    rem -= outsize;
                } while (rem >= outsize);

                strncpy(outbuf, wout, rem);
                bpout = outbuf + rem;
            }

            // is input buffer empty?
            if (bpin > eob)
            {
                write_pending(outbuf, &bpout);

                ssize_t n_read = safe_read(input_desc, inbuf, insize);

                if (n_read <= 0)
                {
                    newlines2 = newlines;
                    return n_read == 0;
                }

                bpin = inbuf;
                eob = inbuf + n_read;
                *eob = '\n';
            }
            else
            {
                if (++newlines > 0)
                {
                    if (newlines >= 2)
                    {
                        newlines = 2;
                        if (sflag)
                        {
                            ch = *bpin++;
                            continue;
                        }
                    }

                    if (nflag && !bflag)
                    {
                        next_line_num();
                        bpout = stpcpy(bpout, line_num_print);
                    }
                }
                // Output a currency symbol if requested
                if (Eflag)
                    *bpout++ = '$';
                // Out put the newline
                *bpout++ = ch;
            }
            ch = *bpin++;
        } while (ch == '\n');

        if (newlines >= 0 && nflag)
        {
            next_line_num();
            bpout = stpcpy(bpout, line_num_print);
        }

        while (true)
        {
            if (ch == '\t' && Tflag)
            {
                *bpout++ = '^';
                *bpout++ = ch + 64;
            }
            else if (ch != '\n')
                *bpout++ = ch;
            else
            {
                newlines = -1;
                break;
            }
            ch = *bpin++;
        }
    }
}

int main(int argc, char **argv)
{
    bool ok = true;

    static struct option const long_options[] =
        {
            {"show-all", no_argument, NULL, 'A'},
            {"number-nonblank", no_argument, NULL, 'b'},
            {"show-ends", no_argument, NULL, 'E'},
            {"number", no_argument, NULL, 'n'},
            {"squeeze-blank", no_argument, NULL, 's'},
            {"show-tabs", no_argument, NULL, 'T'},
            {"show-nonprinting", no_argument, NULL, 'v'},
            {GETOPT_HELP_OPTION_DECL},
            {GETOPT_VERSION_OPTION_DECL},
            {NULL, 0, NULL, 0}};

    // set_program_name (argv[0]);
    (void)setlocale(LC_ALL, "");

    /* Parse command line options. */
    int c;
    while ((c = getopt_long(argc, argv, "AbeEnstTuv", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'A':
            Aflag = 1;
            break;
        case 'b':
            bflag = 1;
            nflag = 1;
            break;
        case 'e':
            eflag = 1;
        case 'E':
            Eflag = 1;
            break;
        case 'n':
            nflag = 1;
            break;
        case 's':
            sflag = 1;
            break;
        case 't':
            tflag = 1;
        case 'T':
            Tflag = 1;
            break;
        case 'u':
            break;
        case 'v':
            vflag = 1;
            break;

            case_GETOPT_HELP_CHAR;

            case_GETOPT_VERSION_CHAR;

        default:
            printf("Try 'cat --help' for more information.\n");
            break;
        }
    }

    /* get optimal blocksize of output. */
    struct stat stat_buf;
    if (fstat(STDOUT_FILENO, &stat_buf) < 0)
    {
        perror("stat error: ");
        exit(1);
    }
    size_t outsize = stat_buf.st_blksize;
    size_t insize;

    /* Main loop */
    char *infile = "-";
    int file_open_mode = O_RDONLY;

    do
    {
        if (optind < argc)
            infile = argv[optind];
        if (!strcmp(infile, "-"))
        {
            input_desc = STDIN_FILENO;
        }
        else
        {
            if ((input_desc = open(infile, file_open_mode)) < 0)
            {
                perror("cannot open file: ");
                ok = false;
                continue;
            }
        }

        if (fstat(input_desc, &stat_buf) < 0)
        {
            perror("stat input file error:");
            ok = false;
            goto contin;
        }

        insize = stat_buf.st_blksize;

        /* Pointer to the input buffer */
        // with an extra byte for a newline sentinel.
        insize = insize < outsize ? outsize: insize;
        char *inbuf = (char *)malloc(insize + 1);

        size_t bufsize = outsize - 1 + insize * 4 + LINE_COUNTER_BUF_LEN;
        char *outbuf = (char *)malloc(bufsize);

        ok &= cat(inbuf, insize, outbuf, outsize);

        free(inbuf);
        free(outbuf);
    contin:

        /* code */
    } while (++optind < argc);

    return 0;
}