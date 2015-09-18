/**
 *
 * @file ringbuffer.c
 * Betriebssysteme ringbuffer.c
 * Beispiel 3
 *
 * @author Andrea Maierhofer	(1410258024)		ic14b024@technikum-wien.at
 * @author Reinhard Mayr    	(1410258030)		ic14b030@technikum-wien.at
 * @author Thomas Schmid		(1410258013)		ic14b013@technikum-wien.at
 * @date 2015/06/07
 *
 * @version $Revision: 28 $
 *
 * @todo nothing right now
 *
 * Last Modified: $Author: Reinhard Mayr $
 *
 **/
 
 
/*
 * -------------------------------------------------------------- review --
 */

/*
 * -------------------------------------------------------------- includes --
 */
#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include<errno.h>
#include "sem182.h"
#include "ringbuffer.h"


/*
 * --------------------------------------------------------------- defines --
 */



 /*
 * --------------------------------------------------------------- globals --
 */
 
/** Program name used in usage and print error for user. */ 
const char* g_program_name;

/** Program mode (receiver or sender). */ 
int g_prog_mode = -1;

/** Semaphore IDs*/
int id_sem_read = -1;
int id_sem_write = -1;

/** Variables for buffer management*/
int buffer_index = 0;
int buffer_size = -1;
int buffer_id = -1;
int *buffer_ptr = NULL;

static const int SIZE_BASE = 10;
/*
 * --------------------------------------------------------------- static --
 */
static void usage(void);

static int create_sem(const int sem_mode, const int sem_value);
static int destroy_sem(const int sem_mode);
static int create_buffer(const int buffer_size);
static int attach_buffer(const int buffer_mode);
static int detach_buffer(void);
static int destroy_buffer(void);
static int cleanup_ressources(const int g_prog_mode);
/*
 * ------------------------------------------------------------- functions --
 */

/**********************************************************************************************************************/
/**
 *
 * \brief get size of ringbuffer.
 *
 * Check program parameters and return size of ringbuffer given by the user.
 *
 * \param argc the number of arguments given by the user.
 * \param argv argument vector of the program including program name as gie.
 *
 * \return size of ringbuffer.
 *
 * \retval -1 indicates an error.
 * \retval > 0 is the ringbuffer size given by the user. 
 *
 **/

int get_buffer_size(const int argc, char* argv[])
{
	int option;
	long int size = -1;
	char* phelper = NULL;

	g_program_name = argv[0];

	errno = 0;

	/* check parameter list for valid options */
	while ((option = getopt(argc, argv, "+m:")) != -1)
	{
		switch (option)
		{
			case 'm':
				/* convert buffer size string to numeric  */
				size = strtol(optarg, &phelper, SIZE_BASE);
				if (errno != 0)
				{
					fprintf(stderr, "%s: %s %s.\n", g_program_name, "could not convert argument", strerror(errno));

					cleanup_ressources(PRG_ERROR);
					usage();
					return RETURN_ERROR;
				}
				/* check whether given size is reasonable  */
				if (option == '\0' || *phelper != '\0' ||
						size <= 0 || size > INT_MAX)
				{
					fprintf(stderr, "%s: %s.\n", g_program_name, "invalid buffer size in argument");

					usage();
					return RETURN_ERROR;
				}
				break;

			case '?':
			default:
				fprintf(stderr, "%s: %s -%c.\n", g_program_name, "invalid option", option);
				usage();
				return RETURN_ERROR;
		}
	}

	/* check for extra parameters */
	if (optind < argc)
	{
		fprintf(stderr, "%s: %s.\n", g_program_name, "illegal number of options");
		usage();
		return RETURN_ERROR;
	}

	return size;
}


/**********************************************************************************************************************/
/**
 *
 * \brief create_environment
 *
 * create_environment
 *
 * creates semaphores and buffer
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

int create_environment(void)
{
	/* semaphore for reading */
	if (create_sem(SEMAPHORE_READ_MODE, 0) == RETURN_ERROR)
	{
		return RETURN_ERROR;
	}

	/* semaphore for writing */
	if (create_sem(SEMAPHORE_WRITE_MODE, buffer_size) == RETURN_ERROR)
	{
		return RETURN_ERROR;
	}

	/* the ring buffer itself  */
	if (create_buffer(buffer_size) == RETURN_ERROR)
	{
		return RETURN_ERROR;
	}
	if (attach_buffer((g_prog_mode == PRG_RECEIVER_MODE)?SHAREDMEMORY_READ_MODE:SHAREDMEMORY_WRITE_MODE) == RETURN_ERROR){
		return RETURN_ERROR;
	}
	
	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief destroy_environment
 *
 * destroy_environment
 *
 * deletes semaphores and buffer
 *
 * \param res_mode - cleanup due to error condition or due to progam "shutdown"
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

int destroy_environment(const int res_mode)
{
	return cleanup_ressources(res_mode);
}

/**********************************************************************************************************************/
/**
 *
 * \brief my_sem_post
 *
 * my_sem_post
 *
 * increments semaphore determined by program mode (g_prog_mode) 
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

int my_sem_post(void)
{
	int sem_id;
	/* get semaphore id depending on being reader or writer */
	sem_id = (g_prog_mode == PRG_SENDER_MODE) ? id_sem_read:id_sem_write;
	errno = 0;
	if (V(sem_id) == -1)
	{
		fprintf(stderr, "%s: %s %s.\n", g_program_name, "my_sem_post could not increment semaphore.", strerror(errno));
		cleanup_ressources(PRG_ERROR);
		return RETURN_ERROR;
	}

	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief my_sem_wait
 *
 * my_sem_wait
 *
 * decrements semaphore determined by program mode (g_prog_mode) 
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

int my_sem_wait(void)
{
	int sem_id;
	/* get semaphore id depending on being reader or writer */
	sem_id = (g_prog_mode == PRG_RECEIVER_MODE) ? id_sem_read : id_sem_write;
	errno = 0;
	while (P(sem_id) == -1)
	{
		if (errno != EINTR)
		{
			fprintf(stderr, "%s: %s %s\n", g_program_name, "my_sem_wait could not decrement semaphore.", strerror(errno));
			cleanup_ressources(PRG_ERROR);
			return RETURN_ERROR;
		}
		errno = 0;
	}

	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief read_char_from_buffer
 *
 * read_char_from_buffer
 *
 * reads one character from ring buffer
 *
 * \return character from buffer
 *
 **/

int read_char_from_buffer(void)
{
	int index;
	index = buffer_index++;
	
	/* Move to the first position of the buffer when the end of buffer was reached */
	/* see Sisyphus legend for details */
	if (buffer_index >= buffer_size) {
		buffer_index = 0;
	}
	return *(buffer_ptr + index);	
}

/**********************************************************************************************************************/
/**
 *
 * \brief write_char_to_buffer
 *
 * write_char_to_buffer
 *
 * writes one character into ring buffer
 *
 * \param character_to_write character to be written
 *
 * \return void
 *
 **/

void write_char_to_buffer(int character_to_write)
{
	/* Write character to shared memory */
	*(buffer_ptr + buffer_index) = character_to_write;
	buffer_index++;

	/* Move to the first position of the buffer when the end of buffer was reached */
	/* see Sisyphus legend for details */
	if (buffer_index >= buffer_size) {
		buffer_index = 0;
	}
}

/**********************************************************************************************************************/
/**
 *
 * \brief usage
 *
 * usage
 *
 * prints usage message to STDOUT
 *  
 * \return void
 **/

static void usage(void)
{
	if (fprintf(stdout, "Usage: %s -m <ringbuffer size>\n", g_program_name) < 0)
	{
		fprintf(stderr, "%s: %s\n", g_program_name, "Error printing usage message to STDOUT.");
		destroy_environment(PRG_SUCCESS);
		exit(EXIT_FAILURE);
	}
}
/**********************************************************************************************************************/
/**
 *
 * \brief create_sem
 *
 * create_sem
 *
 * creates semaphore for read or for write action
 *
 * \param sem_mode = SEMAPHORE_READ_MODE or SEMAPHORE_WRITE_MODE
 * \param sem_value = inital value for semaphore
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

static int create_sem(const int sem_mode, const int sem_value)
{
	int sem_id = -1;
	int sem_key;

	errno = 0;

	/* get semaphore key depending on we being reader or writer */
	sem_key = (sem_mode == SEMAPHORE_READ_MODE) ? KEY_RINGBUFFER_READ_SEMAPHORE : KEY_RINGBUFFER_WRITE_SEMAPHORE;

	/* try to create semaphore */
	if ((sem_id = seminit(sem_key, PERMISSIONS, sem_value)) == -1)
	{
		if (errno == EEXIST)
		{
			errno = 0;
			/* semaphore was created, lets grab it */
			if ((sem_id = semgrab(sem_key)) == -1)
			{
				fprintf(stderr, "%s: %s %s\n", g_program_name, "create_sem could not grab semaphore descriptor.", strerror(errno));
				cleanup_ressources(PRG_ERROR);
				return RETURN_ERROR;
			}
		}
		else
		{
			/* some error happened when creating the semaphore */
			fprintf(stderr, "%s: %s %s\n", g_program_name, "create_sem could not create semaphore.", strerror(errno));
			cleanup_ressources(PRG_ERROR);
			return RETURN_ERROR;
		}
	}

	/* save semaphore id for later use */
	if (sem_mode == SEMAPHORE_READ_MODE){
		id_sem_read = sem_id;
	}
	else
	{
		id_sem_write = sem_id;
	}
	
	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief destroy_sem
 *
 * destroy_sem
 *
 * deletes semaphore for read or for write action
 *
 * \param sem_mode = SEMAPHORE_READ_MODE or SEMAPHORE_WRITE_MODE
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

static int destroy_sem(const int sem_mode)
{
	int sem_id;
	/* get semaphore id depending depending on being reader or writer */
	sem_id = (sem_mode == SEMAPHORE_READ_MODE) ? id_sem_read : id_sem_write;

	/* reset semaphore variable depending on being reader or writer */
	if (sem_mode == SEMAPHORE_READ_MODE)
	{
		id_sem_read = -1;
	}
	else {
		id_sem_write = -1;
	}

	errno = 0;
	/* remove semaphore */
	if (semrm(sem_id) == -1)
	{
		fprintf(stderr, "%s: %s %d. %s\n", g_program_name, "destroy_sem could not remove semaphore with id ", sem_id, strerror(errno));
		return RETURN_ERROR;
	}
	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief create_buffer
 *
 * create_buffer
 *
 * creates ring buffer in shared memory
 *
 * \param buffer_size size of ring buffer
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

static int create_buffer(const int buffer_size)
{
	errno = 0;
	
	/* create shared memory */
	if ((buffer_id = shmget(KEY_RINGBUFFER_READ_SHARED_MEMORY, sizeof(int) * buffer_size, PERMISSIONS | IPC_CREAT)) == -1)
	{
		fprintf(stderr, "%s: %s %s\n", g_program_name, "create_buffer could not get shared memory.", strerror(errno));
		cleanup_ressources(PRG_ERROR);
		return RETURN_ERROR;
	}
	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief attach_buffer
 *
 * attach_buffer
 *
 * Attaches shared memory as read or write buffer, depending on program mode (reader / writer)
 *
 * \param buffer_mode = SHAREDMEMORY_READ_MODE
 *                          SHAREDMEMORY_WRITE_MODE
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

static int attach_buffer(const int buffer_mode)
{
	int shmflg;
	/* set shmflag depending on being reader or writer */
	shmflg = (buffer_mode == SHAREDMEMORY_READ_MODE) ? SHM_RDONLY : 0;
	errno = 0;
	/* attach shared memory */
	if ((buffer_ptr = shmat(buffer_id, NULL, shmflg)) == (void *) -1)
	{
		fprintf(stderr, "%s: %s %s\n", g_program_name, "attach_buffer could not attach shared memory.", strerror(errno));
		cleanup_ressources(PRG_ERROR);
		return RETURN_ERROR;
	}

	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief detach_buffer
 *
 * detach_buffer
 *
 * detaches shared memory
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

static int detach_buffer (void)
{
	errno = 0;
	/* detach shared memory */
	if (shmdt(buffer_ptr) == -1)
	{
		fprintf(stderr, "%s: %s %s\n", g_program_name, "detach_buffer could not detach shared memory.", strerror(errno));
		buffer_ptr = NULL;
		return RETURN_ERROR;
	}	
	buffer_ptr = NULL;
	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief destroy_buffer
 *
 * destroy_buffer
 *
 * deletes shared memory
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

static int destroy_buffer(void)
{
	errno = 0;
	/* send delete command to shared memory control */
	if (shmctl(buffer_id, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "%s: %s %s\n", g_program_name, "destroy_buffer could not remove shared memory.", strerror(errno));
		buffer_id = -1;
		return RETURN_ERROR;
	}
	buffer_id = -1;
	return RETURN_SUCCESS;
}

/**********************************************************************************************************************/
/**
 *
 * \brief cleanup_ressources
 *
 * cleanup_ressources
 *
 * resets static variables, deletes semaphores and buffer depending on program mode (reader / writer)
 *
 * \param res_mode cleanup due to error condition or due to progam "shutdown"
 *
 * \return RETURN_SUCCESS oder RETURN_ERROR
 *
 * \retval 0 = RETURN_SUCCESS
 * \retval -1 = RETURN_ERROR
 *
 **/

static int cleanup_ressources(const int res_mode)
{
	int return_value = RETURN_SUCCESS;

	/* detach buffer if there is one */
	if (buffer_ptr != NULL)
	{
		if (detach_buffer() == RETURN_ERROR)
		{
			return_value = RETURN_ERROR;
		}
	}

	/* cleanup will be only done by reader during "normal shutdown" */
	/* or by anyone of both in case of error */
	if (g_prog_mode == PRG_RECEIVER_MODE || res_mode == PRG_ERROR)
	{
		/* delete buffer */
		if (buffer_id != -1)
		{
			if (destroy_buffer() == RETURN_ERROR)
			{
				return_value = RETURN_ERROR;
			}
		}

		/* delete semaphores */
		if (id_sem_read != -1)
		{
			if (destroy_sem(SEMAPHORE_READ_MODE) == RETURN_ERROR)
			{
				return_value = RETURN_ERROR;
			}
		}
		if (id_sem_write != -1)
		{
			if (destroy_sem(SEMAPHORE_WRITE_MODE) == RETURN_ERROR)
			{
				return_value = RETURN_ERROR;
			}
		}
	}

	/* reset variables */
	g_prog_mode = -1;
	buffer_index = 0;
	buffer_size = -1;

	return return_value;
}
/****************************************************EOF******************************************************************/
