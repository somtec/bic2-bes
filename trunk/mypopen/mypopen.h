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
#define RETURN_FAILURE -1

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
 * \brief This function is a simplified replacement for popen().
 *
 * This functions opens a pipe, forks a child process and invokes the shell in the child process.
 * Since a pipe is by definition unidirectional, the type argument may specify only reading or writing.
 * The command argument is a pointer to a null-terminated string containing a shell command line.
 * This command is passed to /bin/sh using the -c flag.
 * The type argument is a pointer to a null-terminated string which must contain either the letter 'r'
 * for reading or the letter 'w' for writing.
 * The return value from mypopen() is a normal standard I/O stream in all respects save that it
 * must be closed with mypclose().
 *
 * \param command the command to be executed in the shell.
 * \param type "w" parent wants to write to pipe, "r" parent wants to read from pipe.
 *
 * \return NULL if mypopen failed, else return the usable file pointer of the parent.
 */
extern FILE *mypopen(const char *cmd, const char *type);

/**
 * \brief Simplified version of pclose(), used in combination with mypopen.
 *
 * This function closes the pipe, waits for the child process to exit
 * and retrieves and returns the exit status of the child.
 *
 * \param stream file pointer previously opened by mypopen.
 *
 * \return exit status of child process or RETURN_FAILURE in case of an error.
 *
 */
extern int mypclose(FILE *stream);

#endif /* _MYPOPEN_H_ */

/*
 * =================================================================== eof ==
 */
