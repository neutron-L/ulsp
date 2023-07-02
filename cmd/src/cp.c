#include "apue.h"

#define BUFFSIZE 1024
#define COPYMODE 0644

int main(int argc, char **argv)
{
    char buf[BUFFSIZE];
    int in_fd, out_fd, n_chars = 0;
    char *inf = NULL, *outf = NULL;
    int flag = 0;
    int do_write = 1;

    if (argc == 3)
    {
        inf = argv[1];
        outf = argv[2];
        flag = 1;
    }

    else if (argc == 4)
    {
        for (int i = 1; i < 4; ++i)
        {
            if (!strcmp(argv[i], "-i"))
                flag = 1;
            else if (!inf)
                inf = argv[i];
            else
                outf = argv[i];
        }
    }

    if (!flag)
    {
        fprintf(stderr, "usage: cp (-i) <source> <destination>\n");
        exit(1);
    }

    if ((in_fd = open(inf, O_RDONLY)) == -1)
        err_quit("cannot open %s", inf);

    if ((out_fd = open(outf, O_WRONLY)) != -1)
    {
        printf("cp: overwrite '%s'?: ", outf);
        int c = getchar();
        if (c != 'Y' && c != 'y')
            do_write = 0;
    }

    else if ((out_fd = creat(outf, COPYMODE)) == -1)
        err_quit("cannot creat %s", outf);

    if (do_write)
        while ((n_chars = read(in_fd, buf, BUFFSIZE)) > 0)
        {
            if (write(out_fd, buf, n_chars) != n_chars)
                err_quit("Cannot write %s", outf);
        }

    if (n_chars == -1)
        perror("Read error from input file");

    if (close(in_fd) == -1 || close(out_fd) == -1)
        perror("Cannot close file");
    return 0;
}