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
#include <libgen.h>


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
    /** File type pipe e. g. FIFO. */
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
static char* spath_buffer = NULL;

/** Maximum path length of file system */
static long int smax_path = 0;

/** Current program arguments */
static const char* sprogram_arg0 = NULL;

/** Maximum buffer size for print buffer.*/
static const long MAX_PRINT_BUFFER = 1000;

/** Print buffer for printout on stderr. */
static char* sprint_buffer = NULL;

/** Want to convert user id number into decimal number. */
static const int USERID_BASE = 10;

/* ------------------------------------------------------------- functions --
 */

#if DEBUG_OUTPUT
void debug_print(const char* message);
#else /* DEBUG_OUTPUT */
/* suppress debug_print output */
void debug_print(__attribute((unused))const char* message)
{}
#endif /* DEBUG_OUTPUT */

inline static int get_max_path_length(void);
inline static char* get_print_buffer(void);
inline static const char* get_program_argument_0(void);
inline static char* get_path_buffer(void);

void print_usage(void);
int get_current_dir(char current_dir[]);
int do_file(const char* file_name, const char* const * params);
int do_dir(const char* dir_name, const char* const * params);
void print_error(const char* message);
void print_result(const char* file_path);
int init(const char** program_args);
void cleanup(void);
boolean user_exist(const char* user_name);
boolean has_no_user(const struct stat* file_info);
FileType get_file_type(const struct stat* file_info);
FileType get_file_type_info(const char param);
void change_time(const struct stat* file_info);
void filter_name(char* path_to_examine, const char* const * params);
void filter_path(char* path_to_examine, const char* const * params);


/**
 *
 * \brief main implements a a simple replacement for Linux find.
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itself (including the program name in argv[0])
 *
 * \return EXIT_SUCCESS on success  EXIT_FAILURE on error.
 * \retval EXIT_SUCCESS Program ended successfully.
 * \retval EXIT_FAILURE Program ended with failure.
 */
int main(int argc, const char* argv[])
{
    int result = EXIT_SUCCESS;
    char* start_dir = NULL;
    char* found_dir = NULL;
    StatType stbuf;

    result = init(argv);
    if (EXIT_SUCCESS != result)
    {
        cleanup();
        return EXIT_FAILURE;
    }

    if (argc <= 1)
    {
        print_usage();
        return EXIT_SUCCESS;
    }

    get_path_buffer()[0] = '\0';
    start_dir = (char*) malloc(get_max_path_length() * sizeof(char));
    if (NULL == start_dir)
    {
        free(start_dir);
        start_dir = NULL;
        print_error("Out of memory.\n");
        cleanup();
        return EXIT_FAILURE;
    }

    /* build complete path to file (DIR/FILE) */
    snprintf(get_path_buffer(), get_max_path_length(), "%s", argv[1]);

    /*get information about the file and catch errors*/
    if (stat(get_path_buffer(), &stbuf) == -1)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER,
                "Can not read file status of file %s\n", get_path_buffer());
        print_error(get_print_buffer());
    }
    else if (S_ISDIR(stbuf.st_mode))
    {
        found_dir = get_path_buffer();
        start_dir = found_dir;
    }

    /* get current directory */
    if (NULL == found_dir)
    {
        if (NULL == getcwd(start_dir, get_max_path_length()))
        {
            print_error("Can not determine current working directory.");

            free(start_dir);
            start_dir = NULL;
            cleanup();
            /* I/O error */
            return EXIT_FAILURE;
        }
    }
    do_dir(start_dir, argv);

    /* cleanup */
    free(start_dir);
    start_dir = NULL;
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
 * \return void
 *
 */
void debug_print(const char* message)
{
    printf("DGB: %s", message);
}
#endif /* DEBUG_OUTPUT != 0 */

/**
 *
 * \brief Get maximum path length of this Linux file system.
 *
 * \return Maximum path length.
 */
inline static int get_max_path_length(void)
{
    return smax_path;
}

/**
 *
 * \brief Get program argument0 as string.
 *
 * \return Buffer for printing.
 */
inline char* get_print_buffer(void)
{
    return sprint_buffer;
}

/**
 *
 * \brief Get program argument0 as string.
 *
 * \return Program name including path.
 */
inline const char* get_program_argument_0(void)
{
    return sprogram_arg0;
}

/**
 *
 * \brief Get buffer for retrieving the path/directory information.
 *
 * \return Buffer for printing.
 */
inline char* get_path_buffer(void)
{
    return spath_buffer;
}

/**
 *
 * \brief Print the help.
 *
 * \return void
 */
void print_usage(void)
{
    printf("Usage: %s <directory> <test-aktion> ...\n",
            get_program_argument_0());
    printf("Arguments: -user <username|userid>\n");
    printf("           -nouser\n");
    printf("           -type [bcdpfls]\n");
    printf("           -path <glob-pattern>\n");
    printf("           -path <glob-pattern>\n");
    printf("           -print\n");
    printf("           -ls\n");
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
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER,
                "Can not open directory %s\n", dir_name);
        print_error(get_print_buffer());
        return EXIT_SUCCESS;
    }

    while ((dirp = readdir(dirhandle)))
    {
        /*fetch each file from directory, until pointer is NULL*/
        StatType stbuf;
        get_path_buffer()[0] = '\0';

        /* build complete path to file (DIR/FILE) */
        snprintf(get_path_buffer(), get_max_path_length(), "%s/%s", dir_name,
                dirp->d_name);

        /*get information about the file and catch errors*/
        if (stat(get_path_buffer(), &stbuf) == -1)
        {
            snprintf(get_print_buffer(), MAX_PRINT_BUFFER,
                    "Can not read file status of file %s\n", get_path_buffer());
            print_error(get_print_buffer());
        }
        else if (S_ISDIR(stbuf.st_mode))
        {
            filter_name(get_path_buffer(), params);
            filter_path(get_path_buffer(), params);

            if ((strcmp(dirp->d_name, "..") != 0
                    && strcmp(dirp->d_name, ".") != 0))
            {
#if DEBUG_OUTPUT
                snprintf(get_print_buffer(), MAX_PRINT_BUFFER,
                        "Move into directory %s.\n", dirp->d_name);
#endif /* DEBUG_OUTPUT */
                debug_print(get_print_buffer());
                /* recursion for each directory in current directory */
                do_dir(get_path_buffer(), params);
            }
        }
        else
        {
            filter_name(get_path_buffer(), params);
            filter_path(get_path_buffer(), params);
            /* TODO: print the file as it is wanted due to filter */
            fprintf(stdout, "File: %s\n", get_path_buffer());
        }
    }

    if (closedir(dirhandle) < 0)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER,
                "Can not close directory %s\n", dir_name);
        print_error(get_print_buffer());
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
            fprintf(stderr, "%s: %s\n", sprogram_arg0, "Out of memory.\n");
            return ENOMEM;
        }
    }

    if (NULL == spath_buffer)
    {
        /* get maximum directory size */
        smax_path = pathconf(".", _PC_PATH_MAX);
        if (-1 == smax_path)
        {
            print_error("Maximum path length can not be determined.\n");
            return ENODATA;
        }
        spath_buffer = (char*) malloc(smax_path * sizeof(char));
        if (NULL == spath_buffer)
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
    free(spath_buffer);
    spath_buffer = NULL;
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
    fprintf(stderr, "%s: %s\n", get_program_argument_0(), message);
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
    uid = (uid_t) strtol(user_name, &end_userid, USERID_BASE);
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
 *
 * \param File info as from file system.
 *
 * \return The file type enumerator.
 */
FileType get_file_type(const struct stat* file_info)
{

    FileType result = FILE_TYPE_UNKNOWN;

    if (S_ISBLK(file_info->st_mode))
    {
        result = FILE_TYPE_BLOCK;
    }
    else if (S_ISREG(file_info->st_mode))
    {
        result = FILE_TYPE_FILE;
    }
    else if (S_ISCHR(file_info->st_mode))
    {
        result = FILE_TYPE_CHAR;
    }
    else if (S_ISDIR(file_info->st_mode))
    {
        result = FILE_TYPE_DIRECTORY;
    }
    else if (S_ISFIFO(file_info->st_mode))
    {
        result = FILE_TYPE_PIPE;
    }
    else if (S_ISLNK(file_info->st_mode))
    {
        result = FILE_TYPE_LINK;
    }
    else if (S_ISSOCK(file_info->st_mode))
    {
        result = FILE_TYPE_SOCKET;
    }
    return result;

}

/**
 * \brief Query file type of given character.
 *
 * \param Character identifier of file type.
 *
 * \return The file type enumerator.
 */
FileType get_file_type_info(const char param)
{

    FileType result = FILE_TYPE_UNKNOWN;

    switch (param)
    {
    case 'b':
        result = FILE_TYPE_BLOCK;
        break;
    case 'c':
        result = FILE_TYPE_CHAR;
        break;
    case 'd':
        result = FILE_TYPE_DIRECTORY;
        break;
    case 'p':
        result = FILE_TYPE_PIPE;
        break;
    case 'f':
        result = FILE_TYPE_FILE;
        break;
    case 'l':
        result = FILE_TYPE_LINK;
        break;
    case 's':
        result = FILE_TYPE_SOCKET;
        break;
    default:
        result = FILE_TYPE_UNKNOWN;
        break;
    }

    return result;
}

/**
 * \brief Filters the direntry due to -name  parameter.
 *
 * applies -name filter (if defined) to.
 *
 * \param path_to_examine direntry to investigate for name.
 * \param params Program parameter arguments given by user.
 * \return void
 */

void filter_name(char* path_to_examine, const char* const * params)
{
    int i = 1;
    while (params[i]!= NULL) {
        /* If we find a -name Parameter */
        if(strcmp(params[i], PARAM_STR_NAME) == 0)
        {
            /**
             *  We match the actual filepath against the pattern
             *  delivered as argument to -name
             */

            if(fnmatch(params[i+1], basename(path_to_examine), 0)== 0) {
                /* We have a pattern match! */
                print_result(path_to_examine);
            }
            else {
                /* no match */
            }
        }
        i++;
    }
    return;
}

/**
 * \brief Filters the direntry due to -path parameter.
 *
 * applies -name filter (if defined) to.
 *
 * \param path_to_examine direntry to investigate for path.
 * \param params Program parameter arguments given by user.
 * \return void
 */
void filter_path(char* path_to_examine, const char* const * params){
    int i = 1;
    while (params[i]!= NULL) {
        /* If we find a -name Parameter */
        if(strcmp(params[i], PARAM_STR_PATH) == 0)
        {
            /**
             *  We match the actual filepath against the pattern
             *  delivered as argument to -name
             */
            if(fnmatch(params[i+1], basename(path_to_examine), FNM_PATHNAME)== 0) {
                /* We have a pattern match! */
                print_result(path_to_examine);
            }
            else {
                /* no match */
            }
        }
        i++;
    }
    return;
}

/**
 * \brief outputs matching result
 *
 * \param file_path matching file_path.
 * \return void
 */

void print_result(const char* file_path){
    char * current_dir = NULL;
    int length = 0;
    length=get_current_dir(current_dir);
    if(length) {
        /* working path and file_path begin with same part */
        printf(".%s\n",file_path+length*sizeof(char));
    }
    else {
    /* working path and file_pat have no common part */
        printf("%s\n",file_path);
    }
    free(current_dir);
    return ;
}

/**
 * \brief returns length and name of current dir
 *
 * \param buffer_dirname character pointer to string buffer for path or NULL
 *        if buffer_dirname is NULL a buffer will be created and returned in buffer_dirname
 * \return int length of dir in buffer or 0 in case of failure
 */

int get_current_dir(char * buffer_dirname) {
    int need_buffer;
    int buffer_length;
    need_buffer = (buffer_dirname == NULL);
    if(need_buffer) {
        buffer_length = get_max_path_length() * sizeof(char);
        buffer_dirname = (char*) malloc(buffer_length);
    }
    if (NULL == buffer_dirname)
    {
        if(need_buffer) free(buffer_dirname);
        buffer_dirname = NULL;
        print_error("could not allocate memory.\n");
        return 0;
    }
    if (getcwd(buffer_dirname, buffer_length) == NULL) {
        if(need_buffer) free(buffer_dirname);
        buffer_dirname = NULL;
        return 0;
    }
    return strlen(buffer_dirname);
}

/**
 * \brief Print out last changed date of file.
 *
 * \param file_info with the file attributes.
 *
 * \return none
 **/
void change_time(const struct stat* file_info)
{
    /* Convert the time into the local time format it. */
    strftime(get_print_buffer(), MAX_PRINT_BUFFER - 1, "%b %d %H:%M",
            localtime(&file_info->st_mtime));
    fprintf(stdout, "%s", get_print_buffer());
}

/*
 * =================================================================== eof ==
 */

