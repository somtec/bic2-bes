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

#include <limits.h>

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
    NONE, /** myfind without any parameters. */
    USER, /** Only files by this user. */
    NOUSER, /** Only files which have no user. */
    NAME, /** Files with the given filename pattern. */
    PATH, /** Start path where myfind should start. */
    TYPE, /** Type of Linux file system types. */
    LS, /** Print like in Linux LS output. */
    PRINT, /** Print the name of the directory entry. */
    HELP /* Print usage like all Linux bash commands. */
} Parameters;

/**
 The enumeration of filtered file type .
 */
typedef enum FileTypeEnum
{
    /** File type unknown */
    FILE_TYPE_UNKNOWN,
    /** File type block, e. g. hard disk. */
    FILE_TYPE_BLOCK,
    /** File type char e. g. terminal. */
    FILE_TYPE_CHAR,
    /** File type directory. */
    FILE_TYPE_DIRECTORY,
    /** File type pipe e. g. fifo. */
    FILE_TYPE_PIPE,
    /** File type regular file. */
    FILE_TYPE_FILE,
    /** File type link e. g. symbolic link. */
    FILE_TYPE_LINK,
    /** File type socket e. g. stream for communication. */
    FILE_TYPE_SOCKET
} FileType;
/**
 * The struct type for stat return value.
 */
typedef struct stat StatType;

/**
 * The enumeration addition for bool type.
 */
typedef enum booleanEnum
{
    /** Boolean false. */
    FALSE,
    /** Boolean true. */
    TRUE
} boolean;

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * --------------------------------------------------------------- static --
 */

/** Buffer for reading current directory/file. */
static char* sread_path = NULL;

/** Maximum path length of file system */
static long int smax_path = 0;

/** Current program arguments */
static const char* sprogram_arg0 = NULL;

/** Maximum buffer size for print buffer.*/
static const long MAX_PRINT_BUFFER = 1000;

/** Print buffer for printout on stderr. */
static char* sprint_buffer = NULL;

/** Want to convert user id number into decimal number. */
static const int suserid_base = 10;
/* ------------------------------------------------------------- functions --
 */
#if DEBUG_OUTPUT
void debug_print(const char* message);
#else /* DEBUG_OUTPUT */
/* suppress debug_print output */
void debug_print(__attribute((unused))const char* message)
{}
#endif /* DEBUG_OUTPUT */

void print_usage(void);
int do_file(const char* file_name, const char* const * params);
int do_dir(const char* dir_name, const char* const * params);
void print_error(const char* message);
int init(const char** program_args);
void cleanup(void);
boolean user_exist(const char* user_name);
boolean has_no_user(const struct stat* file_info);
FileType get_file_type(const struct stat* file_info);
FileType get_file_type_info(const char param);

void filter_name(const char* pathname, const char* const * params);

/**
 *
 * \brief main implements a a simple replacement for Linux find.
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itself (including the program name in argv[0])
 *
 * \return EXIT_SUCCESS on success  or Posix error number.
 * \retval EXIT_SUCCESS Program ended successfully.
 * \retval ENOMEM Out of memory.
 */
int main(int argc, const char* argv[])
{
    int result = EXIT_SUCCESS;
    char* current_dir;

    /* prevent warnings regarding unused params */
    debug_print("Hello world with debug_print.\n");
    printf("%d %s\n", argc, argv[0]);
    printf("Program finished.\n");

    result = init(argv);
    if (EXIT_SUCCESS != result)
    {
        cleanup();
        return result;
    }
    /* get current directory */

    current_dir = (char*) malloc(smax_path * sizeof(char));
    if (NULL == current_dir)
    {
        free(current_dir);
        current_dir = NULL;

        cleanup();
        return ENOMEM;
    }

    if (NULL == getcwd(current_dir, smax_path))
    {
        print_error("Can not determine current working directory.");

        free(current_dir);
        current_dir = NULL;
        cleanup();
        /* I/O error */
        return EIO;
    }
    do_dir(current_dir, argv);

    /* cleanup */
    free(current_dir);
    current_dir = NULL;
    cleanup();

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
    /* TODO: Reinhard  instead of output "myfind" give out sprogram_arg0 with path stripped off */
    printf("Usage: myfind <path> [arguments].\n");
    printf("Arguments: -user <username|userid>\n");
    printf("           -nouser\n");
    printf("           -type <filetype>\n");
    printf("           -path <pathpattern>\n");
    printf("           -path <namepattern>\n");
    printf("           -print\n");
    printf("           -ls\n");
    printf("           -help\n");
}

/**
 *
 * \brief Iterates through directory.
 *
 * \param dir_name
 * \param params
 * \return void
 */
int do_dir(const char* dir_name, const char* const * params)
{

    DIR* dirhandle = NULL;
    struct dirent* dirp = NULL;

    /*open directory catch error*/
    dirhandle = opendir(dir_name);
    if (NULL == dirhandle)
    {
        snprintf(sprint_buffer, MAX_PRINT_BUFFER, "Can not open directory %s\n",
                dir_name);
        print_error(sprint_buffer);
        return EXIT_SUCCESS;
    }

    while ((dirp = readdir(dirhandle)))
    {
        /*fetch each file from directory, until pointer is NULL*/
        StatType stbuf;
        sread_path[0] = '\0';

        /* build complete path to file (DIR/FILE) */
        snprintf(sread_path, smax_path, "%s/%s", dir_name, dirp->d_name);

        /*get information about the file and catch errors*/
        if (stat(sread_path, &stbuf) == -1)
        {
            snprintf(sprint_buffer, MAX_PRINT_BUFFER, "Can not read file status of file %s\n", sread_path);
            print_error(sprint_buffer);
        } else if (S_ISREG(stbuf.st_mode))
        {
            filter_name(sread_path, params);

            /* TODO: print the file as it is wanted due to filter */
            fprintf(stdout, "File: %s\n", sread_path);
        } else if (S_ISDIR(stbuf.st_mode))
        {
            filter_name((const char *) sread_path, params);

            if ((strcmp(dirp->d_name, "..") != 0
                    && strcmp(dirp->d_name, ".") != 0))
            {
                debug_print("Move into directory");

                fprintf(stdout, "Directory: %s\n", sprint_buffer);
                /* recursion for each directory in current directory */
                do_dir(sread_path, params);
            }
        }
    }

    if (closedir(dirhandle) < 0)
    {
        snprintf(sprint_buffer, MAX_PRINT_BUFFER,
                "Can not close directory %s\n", dir_name);
        print_error(sprint_buffer);
    }

    return EXIT_SUCCESS;

}

/**
 *
 * \brief Handle the file.
 *
 * \param file_name
 * \param params
 * \return void
 */
int do_file(__attribute__((unused))const char* file_name,
        __attribute__((unused)) const char* const * params)
{
    return EXIT_SUCCESS;
}

/**
 * \brief Initializes the program.
 *
 * \param program_args contains the program arguments.
 * \return EXIT_SUCCESS the program was successfully initialized, otherwise program startup failed.
 * \retval ENOMEM posix error out of memory.
 * \retval ENODATA posix error ENODATA no data available for maximum path length.
 *
 */
int init(const char** program_args)
{
    sprogram_arg0 = program_args[0];
    if (NULL == sprint_buffer)
    {
        sprint_buffer = (char*) malloc(MAX_PRINT_BUFFER * sizeof(char));
        if (NULL == sprint_buffer)
        {
            fprintf(stderr, "ERROR in %s: %s", sprogram_arg0,
                    "Out of memory.\n");
            return ENOMEM;
        }
    }

    if (NULL == sread_path)
    {
        /* get maximum directory size */
        smax_path = pathconf(".", _PC_PATH_MAX);
        if (-1 == smax_path)
        {
            print_error("Maximum path length can not be determined.\n");
            return ENODATA;
        }
        sread_path = (char*) malloc(smax_path * sizeof(char));
        if (NULL == sread_path)
        {
            print_error("Out of memory.\n");
            return ENOMEM;
        }
    }

    return EXIT_SUCCESS;

}

/**
 * \brief Cleanup the program.
 *
 * \return void
 */
void cleanup(void)
{
    free(sread_path);
    sread_path = NULL;
    free(sprint_buffer);
    sprint_buffer = NULL;

}

/**
 *
 * \brief Prints error message to stderr.
 *
 * A new line is printed after the message text automatically.
 *
 * \param message output on stderr.
 * \return void
 */
void print_error(const char* message)
{
    fprintf(stderr, "ERROR in %s: %s\n", sprogram_arg0, message);
}

/**
 *\brief Determines if the given user exists.
 *
 * Check if the given user exist as an user or user id on the system.
 *
 *\param user_name to be queried.
 *\
 *\return FALSE user does not exist, TRUE user exists.
 */
boolean user_exist(const char* user_name)
{
    struct passwd* pwd = NULL;
    char* end_userid = NULL;
    uid_t uid = 0;

    pwd = getpwnam(user_name);

    if (NULL != pwd)
    {
        return TRUE;
    }

    /* is it a user id instead of a user name? */
    uid = (uid_t)strtol(user_name, &end_userid, suserid_base);
    if (0 == uid)
    {
        return FALSE;
    }

    pwd = getpwuid(uid);
    if (NULL == pwd)
    {
         return FALSE;
    }

    return TRUE;
}

/**
 * \brief Test if file has no user.
 *
 * \return FALSE File has a user, TRUE file has no user in user id data base.
 */
boolean has_no_user(const struct stat* file_info)
{
    return (NULL != getpwuid(file_info->st_uid));
}

/**
 * \brief Query file type of given file.
 */
FileType get_file_type(const struct stat* file_info)
{

    FileType result = FILE_TYPE_UNKNOWN;

    if (S_ISBLK(file_info->st_mode))
    {
        result = FILE_TYPE_BLOCK;
    }
#if 0
    else if (S_ISREG(file_info->st_mode))
    {

    }
    else if (S_ISREG(file_info->st_mode))
    {

    }


    FILE_TYPE_CHAR,
    /** File type directory. */
    FILE_TYPE_DIRECTORY,
    /** File type pipe e. g. fifo. */
    FILE_TYPE_PIPE,
    /** File type regular file. */
    FILE_TYPE_FILE,
    /** File type link e. g. symbolic link. */
    FILE_TYPE_LINK,
    /** File type socket e. g. stream for communication. */
    FILE_TYPE_SOCKET

#endif /* 0 */
    return result;


}

FileType get_file_type_info(__attribute__((unused))const char param)
{

    FileType result = FILE_TYPE_UNKNOWN;
    return result;
}


/**
 * \brief Filter the files due to givem parameter
 * applies -name filter (if defined) to .
 *
 * !!!!! Five exclamation marks, the sure sign of an insane mind
 *
 * \param params Program parameter arguments.
 * \return void
 */

void filter_name(const char* path_to_find, const char* const * params)
{
	 int i = 1;
	 while (params[i]!= NULL) {
		 printf("examing option %s\n",params[i]);
		 if(strcmp(params[i], PARAM_STR_NAME) == 0)
		 {
			 printf("physical path: %s\n", path_to_find);
			 printf("to compare: %s\n", params[i+1]);

			 if(fnmatch(path_to_find, params[i+1], FNM_PATHNAME)== 0) {
				 printf("%s mathches %s\n", params[i+1],path_to_find);
			 }
			 else {
				 printf("%s NOT MATCHES %s\n", params[i+1],path_to_find);
			 }
		 }
		 i++;
	}

    return;
}

/*
 * =================================================================== eof ==
 */
