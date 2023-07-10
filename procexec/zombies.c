#include <sys/wait.h>
#include <err.h>
#include <libgen.h>

#include "tlsp_hdr.h"

#define CMD_SIZE 200


int 
main(int argc, char **argv)
{
    int num;
    char cmd[CMD_SIZE];

    if (argc > 1)
        num = getInt(argv[1], GN_NONNEG, "num-zombies");
    else
        num = 5;

    printf("Let`s create some zombies!\n");
    snprintf(cmd, CMD_SIZE, "ps | grep %s", basename(argv[0]));
    cmd[CMD_SIZE - 1] = '\0';

    for (int i = 0; i < num; ++i)
    {
        pid_t pid;

        if ((pid = fork()) < 0)
            errExit("fork");
        else if (pid == 0)
            _exit(EXIT_SUCCESS);
        else
            sleep(1);
    }
    system(cmd);

    (void)printf("I'm going to sleep - try to kill my zombie children, if you like.\n");
    sleep(30);
    system(cmd);
    (void)printf("That's enough zombies. Let's have init clean them up.\n");
    (void)printf("Remember to run 'ps a | grep %s' to verify.\n", basename(argv[0]));
    exit(EXIT_SUCCESS);
}