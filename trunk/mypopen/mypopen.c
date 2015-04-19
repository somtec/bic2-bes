/**
 * @file mypopen.c
 * Betriebsysteme popen pclose Modul.
 * Exercise 2
 *
 * @author Andrea Maierhofer (aka Windows fangirl) <andrea.maierhofer@technikum-wien.at>
 * @author Reinhard Mayr <reinhard.mayr@technikum-wien.at>
 * @author Thomas Schmid <thomas.schmid@technikum-wien.at>
 * @date 2015/04/17
 *
 * @version SVN $Revision: 02$*
 *
 */

/*
 * -------------------------------------------------------------- review --
 */

/*
 * -------------------------------------------------------------- includes --
 */
#include <stdlib.h>
#include <errno.h>
#include <paths.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include "mypopen.h"

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
 * --------------------------------------------------------------- static --
 */

/* ------------------------------------------------------------- functions --
 */

/**
 *
 * \brief main implements a a simple replacement for Linux find.
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments.
 * \param argv the arguments itself (including the program name in argv[0]).
 *
 * \return EXIT_SUCCESS on success  EXIT_FAILURE on error.
 * \retval EXIT_SUCCESS Program ended successfully.
 * \retval EXIT_FAILURE Program ended with failure.
 */

static FILE *globalfileptr = NULL;
static pid_t globalpid = -1;

FILE *mypopen(const char *command, const char *type) {
	int thepipe[2];
	int pipe_end_parent = READING_END;
	int pipe_end_child = WRITING_END;

	if (globalfileptr != NULL) {
		errno = EAGAIN;
		return NULL;
	}
	/* check for empty command  */
	if (strlen(command) == 0) {
		errno = EINVAL;
		return NULL;
	}

	/* check for valid operation type */
	if (strcmp(type, "r") != 0 && strcmp(type, "w") != 0) {
		errno = EINVAL;
		return NULL;
	}

	if (type[0] == 'w') {
		pipe_end_parent = WRITING_END;
		pipe_end_child = READING_END;
	}

	/* get pipe, return NULL in case something went wrong */
	if (pipe(thepipe) == RETURN_FAILURE) {
		/* errno will be set by pipe */
		return NULL;
	}
	usleep(FORK_SLEEP_TIME); /* sleep to avoid fork bomb */
	globalpid = fork();
	if (globalpid < 0) {
		/* Error */
		/* TODO: Überlegen ob Errorhandling nötig und wie mit errno umgehen*/
		/* Hinweis: Errno wird auch von close gesetzt */
		close(thepipe[pipe_end_parent]);
		close(thepipe[pipe_end_child]);
		return NULL;
	} else if (globalpid == 0) {

		/* child process*/

		if (close(thepipe[pipe_end_parent]) == RETURN_FAILURE) {
			exit(EXIT_FAILURE);
		}

		if (dup2(thepipe[pipe_end_child], pipe_end_child) != RETURN_FAILURE) {
			if (close(thepipe[pipe_end_child]) != RETURN_FAILURE) {
				execl(_PATH_BSHELL, "sh", "-c", command, NULL);
				/* no return if successfull */
			}
		}
		exit(EXIT_FAILURE);
	} else {
		/* parent  process*/
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
	/* TODO: assert */
	/* the following statement must not be met */
	exit(EXIT_FAILURE);
}

int mypclose(FILE *stream)

{
	int status;
	pid_t pid;

	/* no fp, no child */
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
	return pclose(stream);
}
