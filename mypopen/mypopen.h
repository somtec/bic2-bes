/**
 * @file mypopen.h
 * Betriebsysteme popen Modul.
 * Beispiel 2
 *
 * @author Andrea Maierhofer <ic14b024@technikum-wien.at>
 * @author Reinhard Mayr  <ic14b030@technikum-wien.at>
 * @author Thomas Schmid <ic14b013@technikum-wien.at>
 * 
 * @date 20015/04/17
 *
 * @version $Revision: 1 $
 *
 * @todo Nothing to do
 *
 *
 * Last Modified: $Author:Thomas Schmid $
 */

#ifndef _MYPOPEN_H_
#define _MYPOPEN_H_

/*
 * -------------------------------------------------------------- includes --
 */

#include <stdio.h>

/*
 * --------------------------------------------------------------- defines --
 */
#define READING_END 0
#define WRITING_END 1
#define EXIT_FAILURE 1
#define RETURN_FAILURE -1
#define FORK_SLEEP_TIME 500000

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
 * \brief popen clone
 *
 * This function creates a child process executing the command given in
 * \a cmd, opens a unnamed pipe, redirects stdin (if \a type is equal
 * to "w") or stdout (if \a type is equal to "r") of the child
 * process to the pipe and returns a file pointer to the pipe end, that
 * is not used by the child process to the caller.
 *
 * \param cmd command to execute in the context of the child process
 * \param type "r" or "w" depending on whether the parent process
 *        intends to read from or write to the pipe.
 *
 * \return file pointer to the parent's pipe end or NULL in case of an error
 *
 */
extern FILE *mypopen(const char *cmd, const char *type);

/**
 * \brief pclose clone
 *
 * This function closes the pipe, waits for the child process to exit
 * and retrieves and returns the exit status of the child.
 *
 * \param fp file pointer to parent's pipe end
 *
 * \return exit status of child process or -1 in case of an error
 *
 */
extern int mypclose(FILE *fp);

#endif /* _MYPOPEN_H_ */

/*
 * =================================================================== eof ==
 */
