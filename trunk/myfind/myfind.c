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

/*
 * --------------------------------------------------------------- defines --
 */

/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * ------------------------------------------------------------- functions --
 */

/**
 *
 * \brief Andrea
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
int main(int argc, const char *argv[])
{
    /* prevent warnings regarding unused params */

    printf("Hello world!\n");
    printf("%d\n", argc);
    printf("%s finished.\n", argv[0]);
    return 0;
}

/*
 * =================================================================== eof ==
 */
