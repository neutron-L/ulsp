#include <assert.h>
#include <locale.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/types.h>
#include <dirent.h>

#include "apue.h"
#include "system.h"

#define PROGRAM_NAME "pwd"

static struct option const long_options[] =
    {
        {"physical", no_argument, NULL, 'P'},
        {"logical", no_argument, NULL, 'L'},
        {GETOPT_HELP_OPTION_DECL},
        {NULL, 0, NULL, 0}};

static bool Pflag;

static void
usage(int status)
{
    if (status == EXIT_SUCCESS)
    {
        printf("\npwd - output the current working directory.\n");
        printf("\n   pwd [(-P | --physical)] [(-L | --logical)]\n");
    }
    else
        fprintf(stderr, "(Type '%s --help' for related documentation)\n", PROGRAM_NAME);
    exit(status);
}

static ino_t
get_inode(char *file)
{
    // 获取当前目录的i-node号
    struct stat statbuf;
    if (lstat(file, &statbuf) < 0)
    {
        fprintf(stderr, "%s: cannot stat: %s\n", PROGRAM_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return statbuf.st_ino;
}

static void
get_filename(ino_t ino, char *filename, size_t len)
{
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(".")) == NULL)
    {
        fprintf(stderr, "%s: fail to open directory: %s\n", PROGRAM_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (entry = readdir(dir))
    {
        if (entry->d_ino == ino && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            break;
    }

    if (errno)
    {
        fprintf(stderr, "%s: fail to read directory: %s\n", PROGRAM_NAME, strerror(errno));
        errno = 0;
        goto cls;
    }

    if (entry)
        strncpy(filename, entry->d_name, len);
    else
        strncpy(filename, "\0", len);

cls:
    if (closedir(dir))
    {
        fprintf(stderr, "%s: fail to close directory: %s\n", PROGRAM_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void
do_pwd(ino_t this_inode)
{
    char filename[BUFSIZ] = {'\0'};

    if (get_inode("..") == this_inode)
        goto print;
    // 进入父目录
    chdir("..");

    // 1. 获取当前文件名
    get_filename(this_inode, filename, BUFSIZ);

    this_inode = get_inode(".");
    // 2. 进入父目录递归打印
    do_pwd(this_inode);

    printf("/");
    // 3. 打印当前目录名
print:
    printf("%s", filename);
}

int main(int argc, char **argv)
{
    /* parse arguments */
    int o;
    while ((o = getopt_long(argc, argv, "PL", long_options, NULL)) != -1)
    {
        switch (o)
        {
        case 'L':
            Pflag = false;
            break;
        case 'P':
            Pflag = true;
            break;
        default:
            usage(EXIT_FAILURE);
            break;
        }
    }

    ino_t this_inode, father_inode;

    this_inode = get_inode(".");
    father_inode = get_inode("..");

    if (this_inode == father_inode)
        printf("/\n");
    else
    {
        do_pwd(this_inode);
        printf("\n");
    }

    return EXIT_SUCCESS;
}
