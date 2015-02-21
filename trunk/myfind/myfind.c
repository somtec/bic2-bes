/**
 * @file myfind.c
 * Betriebssysteme Main file for myfind, which is a reduced find version of Linux.
 * Example 1
 *
 * @author Andrea Maierhofer <andrea.maierhofer@technikum-wien.at>
 * @author Reinhard Mayr <reinhard.mayr@technikum-wien.at>
 * @author Thomas Schmid <thomas.schmid@technikum-wien.at>
 * @date 2015/02/15
 *
 * @version SVN $Revision$
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
/** DEBUG_OUTPUT 0 is without debug_print(), else debug_print() function active. */
#define DEBUG_OUTPUT 1

/* supported parameters of myfind */
#define PARAM_STR_USER "-user"
#define PARAM_STR_NOUSER "-nouser"
#define PARAM_STR_NAME "-name"
#define PARAM_STR_PATH "-path"
#define PARAM_STR_TYPE "-type"
#define PARAM_STR_LS "-ls"
#define PARAM_STR_PRINT "-print"
#define PARAM_STR_HELP "-help"
#define PARAM_STR_TYPE_VALS "bcdflps"

/* Output strings if myfind fails. */
#define CHECKSTRINGFORPARAMVALUE_INFO_STR_PARAM "The parameter %s needs correct additional information.\n"
#define CHECKSTRINGFORPARAMVALUE_INFO_STR_PATH "The path is missing.\n"
/*
 * -------------------------------------------------------------- typedefs --
 */

/**
    The enumeration of available parameters for myfind.
*/
typedef enum ParametersEnum
{
   NONE, /** myfind wihthout any parameters. */
   USER, /** Only files by this user. */
   NOUSER, /** Only files which have no user. */
   NAME, /** Files with the given filename pattern. */
   PATH, /** Start path where myfind should start. */
   TYPE, /** Type of linux file system types. */
   LS, /** Print like in Linux LS output. */
   PRINT, /** Print the name of the directory entry. */
   HELP  /* Print usage like all Linux bash commands. */
} Parameters;


/*
 * --------------------------------------------------------------- globals --
 */

/*
 * ------------------------------------------------------------- functions --
 */
#if DEBUG_OUTPUT
void debug_print(const char* message);
#else /* DEBUG_OUTPUT */
/* suppress debug_print output */
void debug_print(__attribute((unused))const char* message) {}
#endif /* DEBUG_OUTPUT */

void print_usage(void);
int do_file(const char* file_name, const char* const params);
int do_dir(const char* dir_name, const char* const * parms);

/**
 *
 * \brief main implements a a simple replacement for Linux find.
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
    printf("%d %s\n", argc, argv[0]);
    printf("Program finished.\n");
    return EXIT_SUCCESS;
}

#if DEBUG_OUTPUT != 0
/**
 *
 * \brief debug_output prints debug messages.
 *
 * Does not append \n to message output.
 *
 * \param message output on stdout.
 * \retval void
 *
 */
void debug_print(const char* message)
{
    printf("DGB: %s", message);
}
#endif /* DEBUG_OUTPUT != 0 */


/**
 *
 * \brief Print the help.
 *
 * \return void
 */
void print_usage(void)
{
    printf("Usage: myfind <path> [arguments].\n");
    printf("Arguments: -user <username|userid>\n");
    printf("           -nouser\n");
    printf("           -type <type>\n");
    printf("           -path <pathpattern>\n");
    printf("           -path <namepattern>\n");
    printf("           -print\n");
    printf("           -ls\n");
    printf("           -help\n");
}



/*
 * =================================================================== eof ==
 */
