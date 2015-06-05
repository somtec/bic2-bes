/**
 *
 * @file sender.c
 * Betriebssysteme Empfaenger File
 * Beispiel 3
 *
 * @author Andrea Maierhofer	(1410258024)		ic14b024@technikum-wien.at
 * @author Reinhard Mayr    	(1410258030)		ic14b030@technikum-wien.at
 * @author Thomas Schmid		(1410258013)		ic14b013@technikum-wien.at
 * @date 2015/06/04
 *
 * @version $Revision: 6 $
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
 * \brief sender
 *
 * Sender Programm
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
	int write_character = -1;

 g_prog_mode  = PRG_SENDER_MODE;
	
    /*check parameters */
	if ((buffer_size = get_buffer_size(argc, argv)) == RETURN_ERROR)
	{
		return EXIT_FAILURE;
	}
    /*create semaphore and ringbuffer*/
	if (create_environment() == RETURN_ERROR)
	{
		return EXIT_FAILURE;
	}
    /*write ringbuffer*/
	do
	{
	    /*read stdin input*/
		write_character = fgetc(stdin);

        /*decrement semaphore*/
		if (my_sem_wait() == RETURN_ERROR)
		{
			return EXIT_FAILURE;
		}
        /*write ringbuffer*/
		write_char_to_buffer(write_character);

        /*increment semaphore*/
		if (my_sem_post() == RETURN_ERROR)
		{
			return EXIT_FAILURE;
		}
	} while (write_character != EOF);

    /*check for errors in input of stdin*/
	if (ferror(stdin))
	{
		fprintf(stderr, "%s: %s%s\n", g_program_name, "Error while reading input from \"stdin\".", strerror(errno));

		destroy_environment(PRG_ERROR);
		return EXIT_FAILURE;
	}
    /*destroy semaphore and ringbuffer*/
	if (destroy_environment(PRG_SUCCESS) == RETURN_ERROR)
	{
		return EXIT_FAILURE;
	}
	else
	{
		return EXIT_SUCCESS;
	}
}

/*****************************************************EOF***************************************************************/


 
