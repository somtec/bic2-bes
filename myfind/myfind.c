/**
 * @file myfind.c
 * Betriebssysteme Main file for myfind, reduced find version in linux.
 * Beispiel 1
 *
 * @author Andrea Maierhofer <andrea.maierhofer@technikum-wien.at>
 * @author Reinhard Mayr <reinhard.mayr@technikum-wien.at>
 * @author Thomas Schmid <thomas.schmid@technikum-wien.at>
 * @date 2015/02/15
 *
 * @version 1
 *
 * @todo Test it more seriously and more complete.
 * @todo Review it for missing error checks.
 * @todo Review it and check the source against the rules at
 *       https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/c-rules.html
 *
 */

/*
 * -------------------------------------------------------------- includes --
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <fnmatch.h>
#include <unistd.h>

/*
 * --------------------------------------------------------------- defines --
 */
/* DEBUG_OUTPUT 0 is without debug_print(), else debug_print() function active. */
#define DEBUG_OUTPUT 1

/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * ------------------------------------------------------------- functions --
 */
#if DEBUG_OUTPUT
void debug_print(const char* message);
#else /* DEBUG_OUTPUT */
void debug_print(__attribute((unused))const char* message) {}
#endif /* DEBUG_OUTPUT */

/**
 *
 * \brief myfind is a imple replacement for linux find.
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return always "success"
 * \retval 0 always
 *
 */
int main(int argc, const char* argv[])
{
    /* prevent warnings regarding unused params */
    debug_print("Hello world with debug_print.\n");
    printf("Program finished.\n");
    return EXIT_SUCCESS;
}

#if DEBUG_OUTPUT != 0
/**
 *
 * \brief debug_output prints debug messages.
 *
 * Does not append \n to message output.
 * .
 * \param message output on stdout.
 * \retval void
 *
 */
void debug_print(const char* message)
{
    printf("DGB: %s", message);
}
#endif /* DEBUG_OUTPUT != 0 */



/*
 * =================================================================== eof ==
 */
