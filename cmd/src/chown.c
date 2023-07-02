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

#define PROGRAM_NAME "chown"

enum
{
    DEREFERENCE_OPTION = CHAR_MAX + 1,
    FROM_OPTION,
    NO_PRESERVE_ROOT,
    PRESERVE_ROOT,
    REFERENCE
};

static struct option const long_options[] =
    {
        {"changes", no_argument, NULL, 'c'},
        {"silent", no_argument, NULL, 'f'},
        {"quiet", no_argument, NULL, 'f'},
        {"verbose", no_argument, NULL, 'v'},
        {"dereference", no_argument, NULL, DEREFERENCE_OPTION},
        {"no-dereference", no_argument, NULL, 'h'},
        {"from", required_argument, NULL, FROM_OPTION},
        {"no-preserve-root", no_argument, NULL, NO_PRESERVE_ROOT},
        {"preserve-root", no_argument, NULL, PRESERVE_ROOT},
        {"reference", required_argument, NULL, REFERENCE},
        {"recursive", no_argument, NULL, 'R'},
        {GETOPT_HELP_OPTION_DECL},
        {GETOPT_VERSION_OPTION_DECL},
        {NULL, 0, NULL, 0}};

static bool Hflag, Lflag, Pflag, cflag, fflag, vflag, hflag, Rflag;
static char *rfile;
static uid_t owner = -1, from_owner = -1;
static gid_t group = -1, from_group = -1;

static void
usage(int status)
{
    if (status == EXIT_SUCCESS)
    {
        printf("Usage: chown [OPTION]... [OWNER][:[GROUP]] FILE...\n");
        printf("  or:  chown [OPTION]... --reference=RFILE FILE...\n");
        printf("Change the owner and/or group of each FILE to OWNER and/or GROUP.\n");
        printf("With --reference, change the owner and group of each FILE to those of RFILE.\n");
        printf("\n");
        printf("  -c, --changes          like verbose but report only when a change is made\n");
        printf("  -f, --silent, --quiet  suppress most error messages\n");
        printf("  -v, --verbose          output a diagnostic for every file processed\n");
        printf("     --dereference      affect the referent of each symbolic link (this is\n");
        printf("                         the default), rather than the symbolic link itself\n");
        printf("  -h, --no-dereference   affect symbolic links instead of any referenced file\n");
        printf("                         (useful only on systems that can change the\n");
        printf("                         ownership of a symlink)\n");
        printf("      --from=CURRENT_OWNER:CURRENT_GROUP\n");
        printf("                         change the owner and/or group of each file only if\n");
        printf("                         its current owner and/or group match those specified\n");
        printf("                         here.  Either may be omitted, in which case a match\n");
        printf("                         is not required for the omitted attribute\n");
        printf("      --no-preserve-root  do not treat '/' specially (the default)\n");
        printf("      --preserve-root    fail to operate recursively on '/'\n");
        printf("      --reference=RFILE  use RFILE's owner and group rather than\n");
        printf("                         specifying OWNER:GROUP values\n");
        printf("  -R, --recursive        operate on files and directories recursively\n");

        printf("\nThe following options modify how a hierarchy is traversed when the -R\n");
        printf("option is also specified.  If more than one is specified, only the final\n");
        printf("one takes effect.\n");
        printf("\n  -H                     if a command line argument is a symbolic link\n");
        printf("                         to a directory, traverse it\n");
        printf("  -L                     traverse every symbolic link to a directory\n");
        printf("                        encountered\n");
        printf("  -P                     do not traverse any symbolic links (default)\n");
        printf("\n");

        printf("      --help     display this help and exit\n");
        printf("      --version  output version information and exit\n");

        printf("\nOwner is unchanged if missing.  Group is unchanged if missing, but changed");
        printf("to login group if implied by a ':' following a symbolic OWNER.\n");
        printf("OWNER and GROUP may be numeric as well as symbolic.\n");
        ;
    }
    else
        fprintf(stderr, "Try '%s --help' for more information.\n", PROGRAM_NAME);
    exit(status);
}

static void
parse_name(const char *str, uid_t *pu, gid_t *pg)
{
    // 根据提供的用户名和组名，找到对应的uid和gid
    char *ug = strdup(str);
    char *colon;
    bool flag = true;
    if (colon = strchr(ug, ':'))
    {
        if (colon == ug)
            flag = false;
        *colon = '\0';
        ++colon;

        if (*colon)
        {
            struct group *grp_info = getgrnam(colon);

            if (grp_info)
                *pg = grp_info->gr_gid;
            else
            {
                fprintf(stderr, "%s: invalid usre: %s\n", PROGRAM_NAME, ug);
                exit(EXIT_FAILURE);
            }
        }
    }
    // 修改uid
    if (flag)
    {
        struct passwd *usr_info = getpwnam(ug);
        if (usr_info)
            *pu = usr_info->pw_uid;
        else
        {
            fprintf(stderr, "%s: invalid usre: %s\n", PROGRAM_NAME, ug);
            exit(EXIT_FAILURE);
        }
    }

    free(ug);
}

static bool
chown_file(const char *filename)
{
    printf("%s\n", filename);
    struct stat statbuf;

    if (stat(filename, &statbuf))
    {
        fprintf(stderr, "%s: failed to get attributes of '%s': %s\n",
                PROGRAM_NAME, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (from_owner != -1 || from_group != -1)
    {

        // 检查文件的uid和gid
        if (from_owner != -1 && statbuf.st_uid != from_owner ||
            from_group != -1 && statbuf.st_gid != from_group)
            return true;
    }

    if (chown(filename, owner, group))
    {
        fprintf(stderr, "%s: fail to chown file '%s': %s\n",
                PROGRAM_NAME, filename, strerror(errno));
        return false;
    }
    else
    {
        bool ok = true;

        // 递归修改文件的属主信息
        if (Rflag && S_ISDIR(statbuf.st_mode))
        {
            DIR *dir;
            struct dirent *entry;

            if ((dir = opendir(filename)) == NULL)
            {
                fprintf(stderr, "%s: cannot access '%s': %s\n", PROGRAM_NAME, filename, strerror(errno));
                ok &= false;
            }

            else
            {
                size_t flen = strlen(filename);
                bool end_slash = filename[flen - 1] == '/';
                int malloc_len = 2 * flen + 2;
                char *child_file = (char *)malloc(malloc_len * sizeof(char));
                while (entry = readdir(dir))
                {
                    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                    {
                        // 构造文件名
                        int clen = strlen(entry->d_name);
                        if (malloc_len <= flen + clen + 2)
                        {
                            malloc_len = flen + clen + 2;
                            free(child_file);
                            child_file = (char *)malloc(malloc_len * sizeof(char));
                        }
                        if (end_slash)
                            sprintf(child_file, "%s%s", filename, entry->d_name);
                        else
                            sprintf(child_file, "%s/%s", filename, entry->d_name);
                        ok &= chown_file(child_file);
                    }
                }

                free(child_file);
            }

            if (closedir(dir))
                ok &= false;
        }
        return ok;
    }
}

int main(int argc, char **argv)
{
    bool ok = true;

    /* Parse arguments*/
    int o;
    while ((o = getopt_long(argc, argv, "cfvhRHLP", long_options, NULL)) != -1)
    {
        switch (o)
        {
        case 'c':
            cflag = true;
            break;
        case 'f':
            cflag = vflag = false;
            break;
        case 'v':
            vflag = true;
            break;
        case 'h':
            hflag = true;
            break;
        case 'R':
            Rflag = true;
            break;
        case 'H':
            break;
        case 'L':
            break;
        case 'P':
            break;

        case DEREFERENCE_OPTION:

        case FROM_OPTION:
            // 解析from的参数
            parse_name(optarg, &from_owner, &from_group);
            break;
        case NO_PRESERVE_ROOT:
        case PRESERVE_ROOT:
        case REFERENCE:
            rfile = optarg;
            break;

            case_GETOPT_HELP_CHAR;
            case_GETOPT_VERSION_CHAR;
        default:
            break;
        }
    }

    /* check arguments */
    if ((argc - optind < (rfile ? 1 : 2)))
    {
        if (argc <= optind)
            fprintf(stderr, "%s: missing operand\n", PROGRAM_NAME);
        else
            fprintf(stderr, "%s: missing operand after '%s'\n", PROGRAM_NAME, argv[argc - 1]);
        usage(EXIT_FAILURE);
    }

    /* get new onwer and group */
    if (rfile)
    {
        struct stat statbuf;

        // 1. get info of rfile
        if (stat(rfile, &statbuf) < 0)
        {
            fprintf(stderr, "%s: failed to get attributes of '%s': %s\n",
                    PROGRAM_NAME, rfile, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // 2. get the uid and gid of reference file
        owner = statbuf.st_uid;
        group = statbuf.st_gid;
    }
    else
    {
        // 根据提供的用户名和组名，找到对应的uid和gid
        parse_name(argv[optind++], &owner, &group);
    }

    /* chown files */
    while (optind < argc)
        ok &= chown_file(argv[optind++]);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}