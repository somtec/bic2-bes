/**
 *
 * @file ringbuffer.h
 * Betriebssysteme ringbuffer.h
 * Beispiel 3
 *
 * @author Andrea Maierhofer	(1410258024)		ic14b024@technikum-wien.at
 * @author Reinhard Mayr    	(1410258030)		ic14b030@technikum-wien.at
 * @author Thomas Schmid		(1410258013)		ic14b013@technikum-wien.at
 * @date 2015/06/04
 *
 * @version $Revision: 3 $
 *
 * @todo nothing right now
 *
 * Last Modified: $Author: Andrea Maierhofer $
 *
 **/

#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H
 
 /*
 * -------------------------------------------------------------- review --
 */

/*
 * -------------------------------------------------------------- includes --
 */


/*
 * --------------------------------------------------------------- defines --
 */
 
/* use unique id for ringbuffer read semaphore 1000 + user id Thomas Schmid on annuminas */ 
#define KEY_RINGBUFFER_READ_SEMAPHORE (1000 * 1823 + 0)

/* use unique id for ringbuffer write semaphore 1001 + user id Thomas Schmid on annuminas */ 
#define KEY_RINGBUFFER_WRITE_SEMAPHORE (1000 * 1823 + 1)

/* use unique id for shared memory semaphore 1002 + user id Thomas Schmid on annuminas */ 
#define KEY_RINGBUFFER_READ_SHARED_MEMORY (1000 * 1823 + 2)

#define PRG_SUCCESS 0
#define PRG_ERROR 1

#define PRG_SENDER_MODE 0 /* program is writer */
#define PRG_RECEIVER_MODE 1 /* program is reader */

#define RETURN_SUCCESS 0
#define RETURN_ERROR -1

#define SHAREDMEMORY_READ_MODE 1 /* shared memory im read mode */
#define SHAREDMEMORY_WRITE_MODE 2 /* shared memory im write mode */

#define SEMAPHORE_READ_MODE  1 /* semaphore reader */
#define SEMAPHORE_WRITE_MODE 2 /* semaphore for writer */

#define PERMISSIONS 0660

 /*
 * --------------------------------------------------------------- globals --
 */
extern const char* g_program_name;
extern int g_prog_mode;
extern int buffer_size;


/*
 * --------------------------------------------------------------- static --
 */

/*
 * ------------------------------------------------------------- functions --
 */

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
extern void print_usage(void);


/**
 *
 * \brief get size of ringbuffer.
 *
 * Check program paramters and return size of ringbuffer given by the user.
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

int get_buffer_size(const int argc, char* argv[]);

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

extern int create_environment(void);

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

extern int destroy_environment(const int res_mode);

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

extern int my_sem_post(void);

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

extern int my_sem_wait(void);

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

extern int read_char_from_buffer(void);

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

extern void write_char_to_buffer(const int character_to_write);


#endif /* __RINGERBUFFER_H */
/**********************************************************EOF********************************************************/
