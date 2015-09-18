/**
 *
 * @file receiver.c
 * Betriebssysteme Empfaenger File
 * Beispiel 3
 *
 * @author Andrea Maierhofer	(1410258024)		ic14b024@technikum-wien.at
 * @author Reinhard Mayr    	(1410258030)		ic14b030@technikum-wien.at
 * @author Thomas Schmid		(1410258013)		ic14b013@technikum-wien.at
 * @date 2015/06/04
 *
 * @version $Revision: 12 $
 *
 * @todo nothing right now
 *
 * Last Modified: $Author: Thomas Schmid $
 *
 **/

/*
 * -------------------------------------------------------------- includes --
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ringbuffer.h"

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

/**********************************************************************************************************************/
/**
 *
 * \brief receiver
 *
 * Receiver Programm
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return EXIT_SUCCESS oder EXIT_FAILURE
 *
 * \retval 0 = EXIT_SUCCESS
 * \retval 1 = EXIT_FAILURE
 *
 **/

int main (int argc, char *argv[])
{
  int read_character = -1;
  
 g_prog_mode = PRG_RECEIVER_MODE;

    /* check parameters */
  	if ((buffer_size = get_buffer_size(argc, argv)) == RETURN_ERROR)
	{ 
		return EXIT_FAILURE;
	}
	/* create semaphore and ringbuffer */
	if (create_environment() == RETURN_ERROR)
	{
		return EXIT_FAILURE;
	}
	
	
	/* read ringbuffer */
	do
	{
        /* decrement semaphore */
	    if (my_sem_wait() == RETURN_ERROR)
		{
			return EXIT_FAILURE;
		}
		/* read ringbuffer */
		read_character = read_char_from_buffer();

        /* increment semaphore */
		if (my_sem_post() == RETURN_ERROR)
		{
			return EXIT_FAILURE;
		}
        
        /* write received content except EOF to output */
		if (read_character != EOF)
		{
			/* write received content to stdout */
			if (fputc(read_character, stdout) == EOF)
			{
				fprintf(stderr, "%s: %s %s\n", g_program_name, "Error while writing output to \"stdout\".", strerror(errno));

				destroy_environment(PRG_ERROR);
				return EXIT_FAILURE;
			}
		}
	} while (read_character != EOF);
		/* write buffered content to stdout */
	if (fflush(stdout) == EOF)
	{
		fprintf(stderr, "%s: %s %s\n", g_program_name, "Error while writing output to \"stdout\".", strerror(errno));

		destroy_environment(PRG_ERROR);
		return EXIT_FAILURE;
	}

	/* destroy semaphore and ringbuffer */
	if (destroy_environment(PRG_SUCCESS) == RETURN_ERROR)
	{
		return EXIT_FAILURE;
	}
	else
	{
		return EXIT_SUCCESS;
	}
	

}

/***************************************************EOF*******************************************************************/

