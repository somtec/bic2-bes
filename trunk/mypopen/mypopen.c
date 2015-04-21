/**
 * @file mypopen.c
 * Betriebsysteme popen pclose Modul.
 * Exercise 2
 *
 * @author Andrea Maierhofer <andrea.maierhofer@technikum-wien.at>
 * @author Reinhard Mayr <reinhard.mayr@technikum-wien.at>
 * @author Thomas Schmid <thomas.schmid@technikum-wien.at>
 * @date 2015/04/17
 *
 * @version SVN $Revision: 04$*
 *
 */

/*
 * -------------------------------------------------------------- review --
 */

/*
 * -------------------------------------------------------------- includes --
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <paths.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include "mypopen.h"

/*
 * --------------------------------------------------------------- defines --
 * define hier rein
*/
#define READING_END 0
#define WRITING_END 1

#define EXIT_FAILURE 1

/* #define FORK_SLEEP_TIME 500000 */

/*
 * -------------------------------------------------------------- typedefs --
 */
/*
 * --------------------------------------------------------------- globals --
 */

/*
 * --------------------------------------------------------------- static --
 */

/* ------------------------------------------------------------- functions --
 */

/** Have to pass globalfileptr from mypopen to mypclose. */
static FILE *globalfileptr = NULL;
/** Have to pass globalpid from mypopen to mypclose. */
static pid_t globalpid = -1;

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
FILE *mypopen(const char *command, const char *type) {
	int thepipe[2];
	/* init pipe ends for reading */
	int pipe_end_parent = READING_END;
	int pipe_end_child = WRITING_END;

	if (globalfileptr != NULL) {
		errno = EAGAIN;
		return NULL;
	}
	/* check for NULL/empty command/type*/
	if ((command == NULL) || (type == NULL) ||
	   strlen(command) == 0) {
			errno = EINVAL;
			return NULL;
		}

	/* check for valid operation type */
	if (strcmp(type, "r") != 0 && strcmp(type, "w") != 0) {
		errno = EINVAL;
		return NULL;
	}

	if (type[0] == 'w') {
		/* we have to set pipe ends for writing */
		pipe_end_parent = WRITING_END;
		pipe_end_child = READING_END;
	}

	/* get pipe, return NULL in case something went wrong */
	if (pipe(thepipe) == RETURN_FAILURE) {
		/* errno will be set by pipe */
		return NULL;
	}
	/* would be sleep to avoid fork bomb, but only 1 fork allowed - not necessary */
	/* usleep(FORK_SLEEP_TIME); */

	globalpid = fork();
	if (globalpid < 0) {
		/* Error */
		/* close will set errno if it fails, but since it only closes a pipe it won't fail */
		/* and even if it would it would still close the pipe */
		close(thepipe[pipe_end_parent]);
		close(thepipe[pipe_end_child]);
		return NULL;
	} else if (globalpid == 0) {

		/* now in child process*/

		if (close(thepipe[pipe_end_parent]) == RETURN_FAILURE) {
			exit(EXIT_FAILURE);
		}

		if (dup2(thepipe[pipe_end_child], pipe_end_child) != RETURN_FAILURE) {
			if (close(thepipe[pipe_end_child]) != RETURN_FAILURE) {
				execl(_PATH_BSHELL, "sh", "-c", command, (char *) NULL);
				/* no return if successfull */
			}
		}
		exit(EXIT_FAILURE);
	} else {
		/* now in parent process*/
		if (close(thepipe[pipe_end_child]) == RETURN_FAILURE) {
			exit(EXIT_FAILURE);
		}
		globalfileptr = fdopen(thepipe[pipe_end_parent], type);
		if (globalfileptr == NULL) {
			close(thepipe[pipe_end_parent]);
			return NULL;
		}
		return globalfileptr;
	}
	/* the following statement must not be met */
	exit(EXIT_FAILURE);
}


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
int mypclose(FILE *stream)

{
	int status;
	pid_t pid;

	/* no filepointer means no child available */
	if (globalfileptr == NULL) {
		errno = ECHILD;
		return RETURN_FAILURE;
	}
	/* check stream, not null and not bogus */
	if (stream == NULL || stream != globalfileptr) {
		errno = EINVAL;
		return RETURN_FAILURE;
	}

	/* try to close fp */
	if (fclose(stream) != 0)
		return RETURN_FAILURE;

	do {
		pid = waitpid(globalpid, &status, 0);
		if (pid == RETURN_FAILURE) {
			if (errno == EINTR)
				continue;
			return RETURN_FAILURE;
		}
	} while (pid != globalpid);

	/* check exit status of child */
	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	}

	/* no status of child, report error */
	errno = ECHILD;
	return RETURN_FAILURE;
}

