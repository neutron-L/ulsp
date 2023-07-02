#ifndef SYSTEM_H
#define SYSTEM_H

#include <limits.h>


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
        exit(EXIT_SUCCESS);         \
        break

#endif
