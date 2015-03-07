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

/** User text string for supported parameter user. */
static const char* PARAM_STR_USER = "-user";
/** User text string for supported parameter nouser. */
static const char* PARAM_STR_NOUSER = "-nouser";
/** User text string for supported parameter name. */
static const char* PARAM_STR_NAME = "-name";
/** Output string for supported parameter path. */
static const char* PARAM_STR_PATH = "-path";
/** User text for supported parameter type. */
static const char* PARAM_STR_TYPE = "-type";
/** User text for supported parameter ls. */
static const char* PARAM_STR_LS = "-ls";

#if 0
/* TODO implement print */
/** User text for supported parameter user. */
static const char* PARAM_STR_PRINT = "-print";
#endif /* 0 */

/** Possible flags set by user for supported parameter type. */
static const char* PARAM_STR_TYPE_VALS = "bcdflps";

#if 0
/* TODO implement parsing of input parameters. */
/** Output strings if parameter can not be determined. */
static const char*  CHECKSTRINGFORPARAMVALUE_INFO_STR_PARAM =
        "The parameter %s needs correct additional information.\n";

/** Output strings if path is missing. */
static const char*  CHECKSTRINGFORPARAMVALUE_INFO_STR_PATH =
        "The path is missing.\n";
#endif /* 0 */


/* ------------------------------------------------------------- functions --
 */

#if DEBUG_OUTPUT
void
debug_print(const char* message);
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
void print_error(const char* message);
int init(const char** program_args);
void cleanup(void);

int get_current_dir(char current_dir[], int* external_buffer_length);
int do_file(const char* file_name, const char* const * params);
int do_dir(const char* dir_name, const char* const* params);
void print_result(const char* file_path, const char* const* params, StatType* file_info);

boolean user_exist(const char* user_name);
int has_no_user(const char* path_to_examine);

FileType get_file_type(const StatType* file_info);
FileType get_file_type_info(const char param);
boolean get_file_stat(const char* path_to_examine, StatType* file_info);

void filter_name(char* path_to_examine, const char* const * params, StatType* file_info);
void filter_path(char* path_to_examine, const char* const * params, StatType* file_info);
void filter_nouser(const char* path_to_examine, const char* const * params, StatType* file_info);
void filter_user(char* path_to_examine, const char* const* params, StatType* file_info);
void filter_type(const char* path_to_examine, const char* const* params, StatType* file_info);


void print_file_change_time(const StatType* file_info);
void print_file_permissions(const StatType* file_info);
void print_usr_and_grp(const StatType* file_info);

void combine_ls(const StatType* file_info);

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
        print_error("malloc() failed: Out of memory.\n");
        cleanup();
        return EXIT_FAILURE;
    }

    /* build complete path to file (DIR/FILE) */
    snprintf(get_path_buffer(), get_max_path_length(), "%s", argv[1]);

    /*get information about the file and catch errors*/
    if (lstat(get_path_buffer(), &stbuf) == -1)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "lstat() failed: Can not read file status of file %s\n", get_path_buffer());
        print_error(get_print_buffer());
    }
    else if (S_ISDIR(stbuf.st_mode))
    {
        found_dir = get_path_buffer();
        strcpy(start_dir, found_dir);
    }

    /* get current directory */
    if (NULL == found_dir)
    {
        if (NULL == getcwd(start_dir, get_max_path_length()))
        {
            print_error("getcwd() failed: Can not determine current working directory.");
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
    printf("Usage: %s <directory> <test-action> ...\n", get_program_argument_0());
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
 *
 * \return void
 */
int do_dir(const char* dir_name, const char* const* params)
{
    DIR* dirhandle = NULL;
    struct dirent* dirp = NULL;

    /*open directory catch error*/
    dirhandle = opendir(dir_name);
    if (NULL == dirhandle)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "opendir() failed: Can not open directory %s\n", dir_name);
        print_error(get_print_buffer());
        return EXIT_SUCCESS;
    }

    errno = 0;
    while ((dirp = readdir(dirhandle)))
    {
        /* fetch each file from directory, until pointer is NULL */
        StatType file_info;

        /* build complete path to file (DIR/FILE) */
        snprintf(get_path_buffer(), get_max_path_length(), "%s/%s", dir_name, dirp->d_name);
        /* get information about the file and catch errors */
        if (lstat(get_path_buffer(), &file_info) == -1)
        {
            snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "lstat() failed: The file %s doesn't exist.\n", get_path_buffer());
            print_error(get_print_buffer());
        }
        else if (S_ISDIR(file_info.st_mode))
        {
            if ((strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, ".") != 0))
            {
                filter_type(get_path_buffer(), params, &file_info);
                filter_name(get_path_buffer(), params, &file_info);
                filter_path(get_path_buffer(), params, &file_info);
                filter_nouser(get_path_buffer(), params, &file_info);
                filter_user(get_path_buffer(), params, &file_info);

#if DEBUG_OUTPUT
                snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "Move into directory %s.\n", dirp->d_name);
#endif /* DEBUG_OUTPUT */
                debug_print(get_print_buffer());
                /* recursion for each directory in current directory */
                do_dir(get_path_buffer(), params);
            }
        }
        else
        {
            filter_type(get_path_buffer(), params, &file_info);
            filter_name(get_path_buffer(), params, &file_info);
            filter_path(get_path_buffer(), params, &file_info);
            filter_nouser(get_path_buffer(), params, &file_info);
            filter_user(get_path_buffer(), params, &file_info);
        }
    }
    if (0 != errno)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "readdir() failed: The dirstream argument is not valid %s\n", dir_name);
        print_error(get_print_buffer());
    }

    if (closedir(dirhandle) < 0)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "closedir() failed: Can not close directory %s\n", dir_name);
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
int do_file(__attribute__((unused))const char* file_name, __attribute__((unused)) const char* const * params)
{
    /* TODO: Andrea wrote: why returning always EXIT_SUCCESS, previously it always returned void */
    return EXIT_SUCCESS;
}

/**
 * \brief Initializes the program.
 *
 * \param program_args contains the program arguments.
 * \return EXIT_SUCCESS the program was successfully initialized, otherwise program startup failed.
 * \retval ENOMEM posix error out of memory.
 * \retval ENODATA posix error ENODATA no data available for maximum path length.
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
            print_error("pathconf() failed: Maximum path length can not be determined.\n");
            return ENODATA;
        }
        spath_buffer = (char*) malloc(smax_path * sizeof(char));
        if (NULL == spath_buffer)
        {
            print_error("malloc() failed: Out of memory.\n");
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
 * \param path_to_examine test if the given file has no user.
 *
 * \return FALSE File has a user, TRUE file has no user in user id data base.
 */
int has_no_user(const char* path_to_examine)
{
    StatType file_info;

    /* TODO  Andrea wrote: why returning -1 in case of stat fails, we can continue program anyway. */

    if (lstat(path_to_examine, &file_info) == -1)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "lstat() failed: Can not read file status o f file %s\n", get_path_buffer());
        print_error(get_print_buffer());
        return -1;
    }
    return (NULL != getpwuid(file_info.st_uid));
}

/**
 * \brief Query file type of given file.
 *
 * \param file_info as from file system.
 *
 * \return FileType the file type enumerator.
 */
FileType get_file_type(const StatType* file_info)
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
 * \param param identifier of file type.
 *
 * \return FileType the file type enumerator.
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
 * \brief Filters the directory entry due to -name  parameter.
 *
 * applies -name filter (if defined) to.
 *
 * \param path_to_examine directory entry to investigate for name.
 * \param params Program parameter arguments given by user.
 * \param file_info as read from operating system.
 *
 * \return void
 */
void filter_name(char* path_to_examine, const char* const * params, StatType* file_info)
{
    int i = 1;

    while (params[i] != NULL)
    {
        /* If we find a -name Parameter */
        if (strcmp(params[i], PARAM_STR_NAME) == 0)
        {
            /**
             *  We match the actual file path against the pattern
             *  delivered as argument to -name
             */

            if (fnmatch(params[i + 1], basename(path_to_examine), 0) == 0)
            {
                /* We have a pattern match! */
                print_result(path_to_examine, params, file_info);
            }
        }
        i++;
    }
    return;
}

/**
 * \brief Filters the directory entry due to -path parameter.
 *
 * applies -name filter (if defined) to.
 *
 * \param path_to_examine directory entry to investigate for path.
 * \param params Program parameter arguments given by user.
 * \param file_info as read from operating system.
 *
 * \return void
 */
void filter_path(char* path_to_examine, const char* const* params, StatType* file_info)
{
    int i = 1;
    while (params[i] != NULL)
    {
        /* If we find a -name Parameter */
        if (strcmp(params[i], PARAM_STR_PATH) == 0)
        {
            /**
             *  We match the actual file path against the pattern
             *  delivered as argument to -name
             */
            if (fnmatch(params[i + 1], basename(path_to_examine), FNM_PATHNAME) == 0)
            {
                /* We have a pattern match! */
                print_result(path_to_examine, params, file_info);
            }
        }
        i++;
    }
    return;
}

/**
 * \brief Filters the directory entry due to -nouser parameter.
 *
 * applies -nouser filter (if defined) to path_to_examine.
 *
 * \param path_to_examine directory entry to investigate for path.
 * \param params Program parameter arguments given by user.
 * \param file_info as read from operating system.
 *
 * \return void
 */
void filter_nouser(const char* path_to_examine, const char* const* params, StatType* file_info)
{
    int i = 1;

    while (params[i] != NULL)
    {
        /* If we find a -nouser Parameter */
        if (strcmp(params[i], PARAM_STR_NOUSER) == 0)
        {
            /**
             *  check if file has no user assigned
             */
            if (has_no_user(path_to_examine) == 1)
            {
                /* No user assigned! */
                print_result(path_to_examine, params, file_info);
            }
            /* other options: 0 = user detected, -1 = Error */

            return; /* processed the option multiple is useless */
        }
        i++;
    }
}

/**
 * \brief Filters the directory entry due to -user parameter.
 *
 * applies -user filter (if defined) to path_to_examine.
 *
 * \param path_to_examine directory entry to investigate for path.
 * \param params Program parameter arguments given by user.
 * \param file_info as read from operating system.
 *
 * \return void
 */
void filter_user(char* path_to_examine, const char* const* params, StatType* file_info)
{
    int i = 1;
    unsigned int search_uid = 0;
    char* end_ptr = NULL;
    struct passwd* pwd = NULL;

    while (params[i] != NULL)
    {
        /* If we find a -nouser Parameter */
        if (strcmp(params[i], PARAM_STR_USER) == 0)
        {
            /**
             *  check if file has assigned the
             *  user/Uid given in -user option
             */
            search_uid = strtol(params[i + 1], &end_ptr, 10);
            if (end_ptr == '\0')
            {
                /* successful string to int conversion */
                /* -> parameter of -user seems to be an UID */
                if (search_uid == file_info->st_gid)
                {
                    print_result(path_to_examine, params, file_info);
                }
            }
            else
            {
                pwd = getpwuid(file_info->st_gid);
                if (strcmp(pwd->pw_name, params[i + 1]) == 0)
                {
                    /* parameter of -user is equal to
                     * user name derived from UID */
                    print_result(path_to_examine, params, file_info);
                }
            }
        }
        i++;
    }
}

/**
 * \brief Filters the directory entry due to -type parameter.
 *
 * applies -type filter (if defined) to path_to_examine.
 *
 * \param path_to_examine directory entry to investigate for path.
 * \param params Program parameter arguments given by user.
 * \param file_info as read from operating system.
 *
 * \return void
 */
void filter_type(const char* path_to_examine, const char* const* params, StatType* file_info)
{
    int i = 1;
    char* option_argument = NULL;
    int test_char = 0;
    const char* parameter1;

    while (params[i] != NULL)
    {
        parameter1 = params[i + 1];
        /* If we find a -nouser Parameter */
        if (strcmp(params[i], PARAM_STR_TYPE) == 0)
        {
            /* check if there is an option argument*/
            if (strlen(params[i + 1]) == 0)
            {
                snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "missing argument to `-%s'", PARAM_STR_TYPE);
                print_error(get_print_buffer());
                return;
            }
            /* check if option argument has only one letter*/
            if (strlen(params[i + 1]) > 1)
            {
                snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "Arguments to %s should contain only one letter",
                PARAM_STR_TYPE);
                print_error(get_print_buffer());
                return;
            }
            /* check if option argument is known */
            test_char = (int) (*parameter1);
            option_argument = strchr(PARAM_STR_TYPE_VALS, test_char);
            if (NULL == option_argument)
            {
                snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "Unknown argument to %s: %c", PARAM_STR_TYPE,
                        *option_argument);
                print_error(get_print_buffer());
                return;
            }

            /* check if option argument describes the same file type as file to examine has */
            if (get_file_type_info(*option_argument) == get_file_type(file_info))
            {
                print_result(path_to_examine, params, file_info);
            }
            return; /* processed the option multiple is useless */
        }
        i++;
    }
}

/**
 * \brief Outputs matching result.
 *
 * \param file_path matching file_path.
 * \param params are the command line arguments.
 * \param file_info as read from operating system.
 *
 * \return void
 */
void print_result(const char* file_path, const char* const* params, StatType* file_info)
{
    char* buffer_current_dir = NULL;
    int buffer_size = 0;
    int length = 0;
    int i = 0;

    /* check for -ls option */
    while (params[i] != NULL)
    {
        if (strcmp(params[i], PARAM_STR_LS) == 0)
        {
            combine_ls(file_info);
            printf(" ");
            break;
        }
        i++;
    }

    length = get_current_dir(buffer_current_dir, &buffer_size);
    if (length)
    {
        /* working path and file_path begin with same part */
        printf(".%s\n", file_path + length * sizeof(char));
    }
    else
    {
        /* working path and file_pat have no common part */
        printf("%s\n", file_path);
    }
    free(buffer_current_dir);
    return;
}

/**
 * \brief Retrieve the file status information.
 *
 * \param path_to_examine matching file_path.
 * \param file_info where to store the status information.
 *
 * \return void
 */
boolean get_file_stat(const char* path_to_examine, StatType* file_info)
{
    /* TODO Andrea wrote: why always returning TRUE? */
    if (lstat(path_to_examine, file_info) == -1)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "Can not read file status of file %s\n", get_path_buffer());
        print_error(get_print_buffer());
    }
    return TRUE;
}

/**
 * \brief Retrieves length and name of current directory.
 *
 * \param buffer_dirname character pointer to string buffer for path or NULL
 *        if buffer_dirname is NULL a buffer will be created and returned in buffer_dirname
 * \param external_buffer_length pointer to integer with length of buffer_dirname
 * \return int length of directory in buffer or 0 in case of failure.
 */
int get_current_dir(char* buffer_dirname, int* external_buffer_length)
{
    int need_buffer = 0;
    int buffer_length = 0;
    need_buffer = (buffer_dirname == NULL);

    if (need_buffer)
    {
        buffer_length = get_max_path_length() * sizeof(char);
        buffer_dirname = (char*) malloc(buffer_length);
        *external_buffer_length = buffer_length;
    }
    else
    {
        buffer_length = *external_buffer_length;
    }

    if (NULL == buffer_dirname)
    {
        if (need_buffer)
            free(buffer_dirname);
        buffer_dirname = NULL;
        print_error("could not allocate memory.\n");
        return 0;
    }
    if (getcwd(buffer_dirname, buffer_length) == NULL)
    {
        if (need_buffer)
            free(buffer_dirname);
        buffer_dirname = NULL;
        return 0;
    }
    return strlen(buffer_dirname);
}

/**
 * \brief Print out last changed date of file on standard output.
 *
 * \param file_info with the file attributes.
 *
 * \return void
 **/
void print_file_change_time(const StatType* file_info)
{
    size_t written = 0;
    /* Convert the time into the local time format it. */
    written = strftime(get_print_buffer(), MAX_PRINT_BUFFER - 1, "%b %d %H:%M", localtime(&file_info->st_mtime));
    if (0 == written)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "strftime() failed: Could not print file changed time\n.");
        print_error(get_print_buffer());
        return;
    }
    fprintf(stdout, "%s", get_print_buffer());
}

/**
 * \brief Print the file permissions on standard output.
 *
 * \param file_info with all file attributes read out from operating system.
 *
 * \return void
 **/
void print_file_permissions(const StatType* file_info)
{
    /* Print file type */
    fprintf(stdout, "%c", get_file_type(file_info));

    /* Print user permissions */
    fprintf(stdout, "%c", (file_info->st_mode & S_IRUSR ? 'r' : '-'));
    fprintf(stdout, "%c", (file_info->st_mode & S_IWUSR ? 'w' : '-'));

    if (!(file_info->st_mode & S_ISUID)) /*no UID-Bit */
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_IXUSR ? 'x' : '-'));
    }
    else if ((file_info->st_mode & S_ISUID) && (file_info->st_mode & S_IXUSR)) /*UID-Bit && Execute-Bit*/
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_ISUID ? 's' : '-'));
    }
    else /*UID-Bit && !Execute-Bit*/
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_ISUID ? 'S' : '-'));
    }

    /* Print group permissions */
    fprintf(stdout, "%c", (file_info->st_mode & S_IRGRP ? 'r' : '-'));
    fprintf(stdout, "%c", (file_info->st_mode & S_IWGRP ? 'w' : '-'));

    if (!(file_info->st_mode & S_ISGID)) /*no GID-Bit */
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_IXGRP ? 'x' : '-'));
    }
    else if ((file_info->st_mode & S_ISGID) && (file_info->st_mode & S_IXGRP)) /*GID-Bit && Execute-Bit */
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_ISGID ? 's' : '-'));
    }
    else /*GID-Bit && !Execute-Bit*/
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_ISGID ? 'S' : '-'));
    }

    /* Print other permissions */
    fprintf(stdout, "%c", (file_info->st_mode & S_IROTH ? 'r' : '-'));
    fprintf(stdout, "%c", (file_info->st_mode & S_IWOTH ? 'w' : '-'));

    if (!(file_info->st_mode & S_ISVTX)) /*Sticky-Bit*/
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_IXOTH ? 'x' : '-'));
    }
    else if ((file_info->st_mode & S_ISVTX) && (file_info->st_mode & S_IXOTH)) /*Sticky-Bit && Execute-Bit*/
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_ISVTX ? 't' : '-'));
    }
    else /*Sticky-Bit && !Execute-Bit*/
    {
        fprintf(stdout, "%c", (file_info->st_mode & S_ISVTX ? 'T' : '-'));
    }

    fprintf(stdout, "  ");
}
/**
 * \brief print username and grpname to standard out
 *
 * \param file_info Struktur mit den Attributen der Datei
 *
 * \return none
 * \retval none
 **/
void print_usr_and_grp(const  StatType* file_info)
{
    struct passwd *pwd;		/* Pointer auf struct passwd in pwd.h */
    struct group *grp;		/* Pointer auf struct group in grp.h */


	/*********** Print username **********************/

    pwd = getpwuid(file_info->st_uid);
    if(pwd != NULL)
    {
    	fprintf(stdout,"%s", pwd->pw_name);
    }
    else
    {
    	fprintf(stdout,"%d", file_info->st_uid);
    }

    fprintf(stdout," ");	/*Leerzeichen*/

	/*********** Print groupname **********************/
    grp = getgrgid(file_info->st_gid);
    if(pwd != NULL)
    {
    fprintf(stdout,"%s", grp->gr_name);
    }
    else if (grp != NULL)
    {
    	fprintf(stdout,"%s", grp->gr_name);
    }
    else
    {
    	fprintf(stdout,"%d", file_info->st_gid);
    }
}
/**
 * \brief -ls Argument returns number of i-nodes,blocks, permissions,
 number of links, owner, group, last modification time and directory name.
 symlinks follow.
 *
 * \param file_info with all file attributes read out from operating system.
 *
 * \return void
 **/
void combine_ls(const StatType* file_info)
{
    /* Print i-node */
    fprintf(stdout, "%8lu    ", (unsigned long) file_info->st_ino);

    /* Print blocks */
    fprintf(stdout, "%2lu  ", (unsigned long) file_info->st_blocks / 2);

    /* Print permissions */
    print_file_permissions(file_info);

    /* Print number of hard links */
    fprintf(stdout, "%2lu ", (unsigned long) file_info->st_nlink);

    /* Print user and group */
    print_usr_and_grp(file_info);

    /* Print file size */
    fprintf(stdout, "%9lu  ", (unsigned long) file_info->st_size);

    /* Print file change time */
    print_file_change_time(file_info);
}
/*
 * =================================================================== eof ==
 */

