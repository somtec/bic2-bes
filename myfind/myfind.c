/**
 * @file myfind.c
 * Betriebssysteme Main file for myfind, which is a reduced find version of Linux.
 * Example 1
 *
 * @author Andrea Maierhofer (aka Windows fangirl) <andrea.maierhofer@technikum-wien.at>
 * @author Reinhard Mayr <reinhard.mayr@technikum-wien.at>
 * @author Thomas Schmid <thomas.schmid@technikum-wien.at>
 * @date 2015/02/15
 *
 * @version SVN $Revision: 64$
 *
 * TODO Test it more seriously and more complete.
 * TODO Review it for missing error checks.
 * TODO Review it and check the source against the rules at
 *       https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/c-rules.html
 *
 */

/*
 * -------------------------------------------------------------- includes --
 */
/* TODO printf and fprintf error handling is not implemented till now. */
/* TODO abort operation if command is parsed successfully or give an error if there /
   are superfluous arguments.
*/
/* TODO
- Unused Parameter in den Filter*()- Funktionen entfernen __attribute((unused)) eliminieren;
- Parsen und herausfinden von falschen  Argumente von Links nach rechts damit die Fehlerausgabe passt und entsprechend das Programm an dieser Stelle abbricht.
  Wenn der Parameter exit gesetzt ist, das Programm direkt in cleanup(TRUE) beenden mit exit(EXIT_FAILURE).
- Review unseres Programms und fehlendes Errorhandling einbauen
  Abfragen der globalen Variablen errno nach bestimmten Systemaufrufen;
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
#define DEBUG_OUTPUT 0

/*
 * -------------------------------------------------------------- typedefs --
 */

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

/** Prototype for printing output functions */
typedef void (*print_detail)(const char* file_path, StatType* file_info);

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * --------------------------------------------------------------- static --
 */

/** Buffer for reading current directory/file. */
static char* spath_buffer = NULL;

/** Buffer for getting the base name of a given directory. */
static char* sbasename_buffer = NULL;

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

/** Index of -user argument, -1 if not present. */
static int sargument_index_user = -1;
/** Index of -nouser argument, -1 if not present. */
static int sargument_index_nouser = -1;
/** Index of -name argument, -1 if not present. */
static int sargument_index_name = -1;
/** Index of -path argument, -1 if not present. */
static int sargument_index_path = -1;
/** Index of -type argument, -1 if not present. */
static int sargument_index_type = -1;
/** Index of -ls argument, -1 if not present. */
static int sargument_index_ls = -1;
/** Index of -print argument, -1 if not present. */
static int sargument_index_print = -1;

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
/** Possible flags set by user for supported parameter type. */
static const char* PARAM_STR_TYPE_VALS = "bcdflps";
/** User text for supported parameter ls. */
static const char* PARAM_STR_LS = "-ls";
/** User text for supported parameter user. */
static const char* PARAM_STR_PRINT = "-print";

/* ------------------------------------------------------------- functions --
 */

#if DEBUG_OUTPUT
void
debug_print(const char* message);
#else /* DEBUG_OUTPUT */
/* suppress debug_print output */
void debug_print(__attribute((unused))const char* message)
{
}
#endif /* DEBUG_OUTPUT */

inline static int get_max_path_length(void);
inline static char* get_print_buffer(void);
inline static const char* get_program_argument_0(void);
inline static char* get_path_buffer(void);

inline static int get_argument_index_user(void);
inline static int get_argument_index_nouser(void);
inline static int get_argument_index_name(void);
inline static int get_argument_index_path(void);
inline static int get_argument_index_type(void);
inline static int get_argument_index_ls(void);
inline static int get_argument_index_print(void);
inline static void set_argument_index_user(int);
inline static void set_argument_index_nouser(int);
inline static void set_argument_index_name(int);
inline static void set_argument_index_path(int);
inline static void set_argument_index_type(int);
inline static void set_argument_index_ls(int);
inline static void set_argument_index_print(int);

static void print_usage(void);
static void print_error(const char* message);
static int init(const char** program_args);
static void cleanup(boolean exit);

static int do_file(const char* file_name, StatType* file_info, const char* const * params);
static int do_dir(const char* dir_name, const char* const * params);
static void print_result(const char* file_path, StatType* file_info);

static boolean user_exist(const char* user_name);
static boolean has_no_user(StatType* file_info);

static char get_file_type(const StatType* file_info);

static boolean filter_name(const char* path_to_examine, const char* const * params,
        StatType* file_info);
static boolean filter_path(const char* path_to_examine, const char* const * params,
        __attribute__((unused)) StatType* file_info);
static boolean filter_nouser(const char* path_to_examine, const char* const * params,
        StatType* file_info);
static boolean filter_user(const char* path_to_examine, const char* const * params,
        StatType* file_info);
static boolean filter_type(const char* path_to_examine, const char* const * params,
        StatType* file_info);

static void print_file_change_time(const StatType* file_info);
static void print_file_permissions(const StatType* file_info);
static void print_user_group(const StatType* file_info);

static void print_detail_ls(const char* file_path, StatType* file_info);
static void print_detail_print(const char* file_path, __attribute__((unused)) StatType* file_info);
static void combine_ls(const StatType* file_info);


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
    int result = EXIT_FAILURE;
    char* start_dir = NULL;
    char* found_dir = NULL;
    StatType stbuf;
    int current_argument = 1; /* the first argument is the program name anyway */
    int test_char = '\0';

    result = init(argv);
    if (EXIT_SUCCESS != result)
    {
        cleanup(TRUE);
    }

    if (argc <= 1)
    {
        print_usage();
        return EXIT_SUCCESS;
    }

    /* check the input arguments first */
    while (current_argument < argc)
    {
        if (0 == strcmp(PARAM_STR_USER, argv[current_argument]))
        {
            /* found -user */
            if ((current_argument + 1) < argc)
            {
                set_argument_index_user(current_argument);
                ++current_argument;
            }
            else
            {
                print_error("Missing argument to '-user'.\n");
                cleanup(TRUE);
            }
        }

        if (0 == strcmp(PARAM_STR_NOUSER, argv[current_argument]))
        {
            /* found -nouser */
            set_argument_index_nouser(current_argument);
        }

        if (0 == strcmp(PARAM_STR_LS, argv[current_argument]))
        {
            /* found -ls */
            set_argument_index_ls(current_argument);
        }

        if (0 == strcmp(PARAM_STR_PRINT, argv[current_argument]))
        {
            /* found -print */
            set_argument_index_print(current_argument);
        }

        if (0 == strcmp(PARAM_STR_NAME, argv[current_argument]))
        {
            /* found -name */
            if (argc > (current_argument + 1))
            {
                set_argument_index_name(current_argument);
                ++current_argument;
            }
            else
            {
                print_error("Missing argument to '-name'.\n");
                cleanup(TRUE);
            }
        }

        if (0 == strcmp(PARAM_STR_PATH, argv[current_argument]))
        {
            /* found -path */
            if (argc > (current_argument + 1))
            {
                set_argument_index_path(current_argument);
                ++current_argument;
            }
            else
            {
                print_error("Missing argument to '-path'.\n");
                cleanup(TRUE);
            }
        }

        if (0 == strcmp(PARAM_STR_TYPE, argv[current_argument]))
        {
            /* found -type */
            if (argc > (current_argument + 1))
            {
                const char* next_argument = argv[current_argument + 1];
                if (strlen(next_argument) > 1)
                {
                    snprintf(get_print_buffer(), MAX_PRINT_BUFFER,
                            "Argument of -type must be one character of these '%s'\n.", PARAM_STR_TYPE_VALS);
                    print_error(get_print_buffer());
                    return EXIT_FAILURE;
                }

                test_char = (int) (*next_argument);
                if (NULL == strchr(PARAM_STR_TYPE_VALS, test_char))
                {
                    snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "Argument -type unknown options of %s: %c",
                            PARAM_STR_TYPE, *next_argument);
                    print_error(get_print_buffer());
                    return EXIT_FAILURE;
                }
                set_argument_index_type(current_argument);
                ++current_argument;
            }
            else
            {
                print_error("Missing argument to '-type'.\n");
                cleanup(TRUE);
            }
        }

        ++current_argument;
    }

    /* determine the directory for start */
    get_path_buffer()[0] = '\0';
    start_dir = (char*) malloc(get_max_path_length() * sizeof(char));
    if (NULL == start_dir)
    {
        free(start_dir);
        start_dir = NULL;
        print_error("malloc() failed: Out of memory.\n");
        cleanup(TRUE);
    }

    /* build complete path to file (DIR/FILE) */
    snprintf(get_path_buffer(), get_max_path_length(), "%s", argv[1]);

    /*get information about the file and catch errors*/
    if (-1 != lstat(get_path_buffer(), &stbuf))
    {
        if (S_ISDIR(stbuf.st_mode))
        {
            found_dir = get_path_buffer();
            strcpy(start_dir, found_dir);
        }
    }


    if (NULL == found_dir)
    {
        /* use current directory */
        strcpy(start_dir,".");
    }
    result = do_dir(start_dir, argv);

    /* cleanup */
    free(start_dir);
    start_dir = NULL;
    cleanup(FALSE);

    return result;
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
inline static char* get_print_buffer(void)
{
    return sprint_buffer;
}

/**
 *
 * \brief Get program argument0 as string.
 *
 * \return Program name including path.
 */
inline static const char* get_program_argument_0(void)
{
    return sprogram_arg0;
}

/**
 *
 * \brief Get buffer for retrieving the path/directory information.
 *
 * \return Buffer for path information.
 */
inline static char* get_path_buffer(void)
{
    return spath_buffer;
}

/**
 *
 * \brief Get buffer where to write the basename of a path.
 *
 * \return Buffer for path information.
 */
inline static char* get_base_name_buffer(void)
{
    return sbasename_buffer;
}

/**
 *
 * \brief Get index of -user argument.
 *
 * \return -1 if argument not present, else index of argument.
 */
inline static int get_argument_index_user(void)
{
    return sargument_index_user;
}

/**
 *
 * \brief Get index of -nouser argument.
 *
 * \return -1 if argument not present, else index of argument.
 */
inline static int get_argument_index_nouser(void)
{
    return sargument_index_nouser;
}

/**
 *
 * \brief Get index of -name argument.
 *
 * \return -1 if argument not present, else index of argument.
 */
inline static int get_argument_index_name(void)
{
    return sargument_index_name;
}

/**
 *
 * \brief Get index of -path argument.
 *
 * \return -1 if argument not present, else index of argument.
 */
inline static int get_argument_index_path(void)
{
    return sargument_index_path;
}

/**
 *
 * \brief Get index of -type argument.
 *
 * \return -1 if argument not present, else index of argument.
 */
inline static int get_argument_index_type(void)
{
    return sargument_index_type;
}

/**
 *
 * \brief Get index of -ls argument.
 *
 * \return -1 if argument not present, else index of argument.
 */
inline static int get_argument_index_ls(void)
{
    return sargument_index_ls;
}

/**
 *
 * \brief Get index of -print argument.
 *
 * \return -1 if argument not present, else index of argument.
 */
inline static int get_argument_index_print(void)
{
    return sargument_index_print;
}

/**
 *
 * \brief Set index of -user argument.
 *
 * \param index to be set.
 *
 * \return void.
 */
inline static void set_argument_index_user(int index)
{
    sargument_index_user = index;
}

/**
 *
 * \brief Set index of -nouser argument.
 *
 * \param index to be set.
 *
 * \return void.
 */
inline static void set_argument_index_nouser(int index)
{
    sargument_index_nouser = index;
}

/**
 *
 * \brief Set index of -name argument.
 *
 * \param index to be set.
 *
 * \return void.
 */
inline static void set_argument_index_name(int index)
{
    sargument_index_name = index;
}

/**
 *
 * \brief Set index of -path argument.
 *
 * \param index to be set.
 *
 * \return void.
 */
inline static void set_argument_index_path(int index)
{
    sargument_index_path = index;
}

/**
 *
 * \brief Set index of -type argument.
 *
 * \param index to be set.
 *
 * \return void.
 */
inline static void set_argument_index_type(int index)
{
    sargument_index_type = index;
}

/**
 *
 * \brief Set index of -ls argument.
 *
 * \param index to be set.
 *
 * \return void.
 */
inline static void set_argument_index_ls(int index)
{
    sargument_index_ls = index;
}

/**
 *
 * \brief Set index of -print argument.
 *
 * \param index to be set.
 *
 * \return void.
 */
inline static void set_argument_index_print(int index)
{
    sargument_index_print = index;
}

/**
 *
 * \brief Print the usage.
 *
 * \return void
 */
static void print_usage(void)
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
static int do_dir(const char* dir_name, const char* const * params)
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
        if (-1 == lstat(get_path_buffer(), &file_info))
        {
            snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "lstat() failed: The file %s doesn't exist.\n",
                    get_path_buffer());
            print_error(get_print_buffer());
            /* check next file */
            continue;
        }

        if (S_ISDIR(file_info.st_mode))
        {
            if ((strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, ".") != 0))
            {
                char* next_path = NULL;
                do_file(get_path_buffer(), &file_info, params);

#if DEBUG_OUTPUT
                snprintf(get_print_buffer(), MAX_PRINT_BUFFER,
                        "Move into directory %s.\n", dirp->d_name);
#endif /* DEBUG_OUTPUT */
                debug_print(get_print_buffer());
                /* recursion for each directory in current directory */
                next_path = (char*) malloc(get_max_path_length() * sizeof(char));
                if (NULL == next_path)
                {
                    print_error("malloc() failed: Out of memory.\n");
                    if (closedir(dirhandle) < 0)
                    {
                        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "closedir() failed: Can not close directory %s\n", dir_name);
                        print_error(get_print_buffer());
                    }
                    return EXIT_FAILURE;
                }
                strcpy(next_path, get_path_buffer());
                if (EXIT_FAILURE == do_dir(next_path, params))
                {
                    if (closedir(dirhandle) < 0)
                    {
                        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "closedir() failed: Can not close directory %s\n", dir_name);
                        print_error(get_print_buffer());
                    }
                    free(next_path);
                    return EXIT_FAILURE;
                }
                free(next_path);
            }
        }
        else
        {
            do_file(get_path_buffer(), &file_info, params);
        }
    }
    if (0 != errno)
    {
        snprintf(get_print_buffer(), MAX_PRINT_BUFFER, "readdir() failed: The dirstream argument is not valid %s\n",
                dir_name);
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
 * \param params is the program argument vector.
 * \return void
 */
static int do_file(const char* file_name, StatType* file_info, const char* const * params)
{
    if (filter_type(file_name, params, file_info) && filter_name(file_name, params, file_info)
            && filter_path(file_name, params, file_info) && filter_nouser(file_name, params, file_info)
            && filter_user(file_name, params, file_info))
    {
        print_result(file_name, file_info);
    }

    return EXIT_SUCCESS;
}

/**
 * \brief Initializes the program.
 *
 * \param program_args contains the program arguments.
 * \return EXIT_SUCCESS the program was successfully initialized,
 *  otherwise program startup failed.
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

    /* get maximum directory size */
    smax_path = pathconf(".", _PC_PATH_MAX);
    if (-1 == smax_path)
    {
        smax_path = 0;
        print_error("pathconf() failed: Maximum path length can not be determined.\n");
        return ENODATA;
    }

    if (NULL == spath_buffer)
    {
        spath_buffer = (char*) malloc(smax_path * sizeof(char));
        if (NULL == spath_buffer)
        {
            print_error("malloc() failed: Out of memory.\n");
            return ENOMEM;
        }
    }
    if (NULL == sbasename_buffer)
    {
        sbasename_buffer = (char*) malloc(smax_path * sizeof(char));
        if (NULL == sbasename_buffer)
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
 * \params exit_program when set exit program immediately with EXIT_FAILURE.
 *
 * \return void
 */
void cleanup(boolean exit_program)
{
    free(spath_buffer);
    spath_buffer = NULL;

    free(sbasename_buffer);
    sbasename_buffer = NULL;

    free(sprint_buffer);
    sprint_buffer = NULL;

    fflush(stderr);
    fflush(stdout);

    if (exit_program)
    {
        exit(EXIT_FAILURE);
    }

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
static boolean user_exist(const char* user_name)
{
    struct passwd* pwd = NULL;
    char* end_userid = NULL;
    uid_t uid = 0;

    pwd = getpwnam(user_name);

    if (NULL != pwd)
    {
        /* the user exist */
        return TRUE;
    }

    /* is it a user id instead of a user name? */
    errno = 0;
    uid = (uid_t) strtol(user_name, &end_userid, USERID_BASE);
    if (errno)
    {
        return FALSE;
    }
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
static boolean has_no_user(StatType* file_info)
{
    return (NULL != getpwuid(file_info->st_uid));
}

/**
 * \brief Query file type of given file.
 *
 * \param file_info as from file system.
 *
 * \return char representing file type .
 */
static char get_file_type(const StatType* file_info)
{

    char result = '-';

    if (S_ISBLK(file_info->st_mode))
    {
        result = 'b';
    }
    else if (S_ISREG(file_info->st_mode))
    {
        result = 'f';
    }
    else if (S_ISCHR(file_info->st_mode))
    {
        result = 'c';
    }
    else if (S_ISDIR(file_info->st_mode))
    {
        result = 'd';
    }
    else if (S_ISFIFO(file_info->st_mode))
    {
        result = 'p';
    }
    else if (S_ISLNK(file_info->st_mode))
    {
        result = 'l';
    }
    else if (S_ISSOCK(file_info->st_mode))
    {
        result = 's';
    }
    return result;

}

#if 0
/**
 * \brief Query file type of given character.
 *
 * \param param identifier of file type.
 *
 * \return FileType the file type enumerator.
 */
static FileType get_file_type_info(const char param)
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
#endif

/**
 * \brief Filters the directory entry due to -name  parameter.
 *
 * applies -name filter (if defined) to.
 *
 * \param path_to_examine directory entry to investigate for name.
 * \param params Program parameter arguments given by user.
 * \param file_info as read from operating system.
 *
 * \return boolean TRUE name filter matched or not given, FALSE no match found.
 */
static boolean filter_name(const char* path_to_examine, const char* const * params,
        __attribute__((unused))  StatType* file_info)
{
    char* buffer = NULL;

    if (get_argument_index_name() < 0)
    {
        return TRUE;
    }
    /*  We match the actual file path against the pattern
     *  delivered as argument to -name
     */
    buffer = strcpy(get_base_name_buffer(), path_to_examine);
    buffer = basename(buffer);
    /* Do we have a pattern match? */
    return (0 == fnmatch(params[get_argument_index_name() + 1], buffer, 0));
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
 * \return boolean TRUE name filter matched or not given, FALSE no match found.
 */
static boolean filter_path(const char* path_to_examine, const char* const * params,
        __attribute__((unused)) StatType* file_info)
{
    char* buffer = NULL;

    if (get_argument_index_path() < 0)
    {
        return TRUE;
    }

    /**
     *  We match the actual file path against the pattern
     *  delivered as argument to -name
     */
    buffer = strcpy(get_base_name_buffer(), path_to_examine);
    buffer = basename(buffer);

    /* Do we have a pattern match? */
    return (0 == fnmatch(params[get_argument_index_path() + 1], buffer, FNM_PATHNAME));
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
 * \return boolean TRUE name filter matched or not given, FALSE no match found.
 */
static boolean filter_nouser(__attribute__((unused)) const char* path_to_examine,
        __attribute__((unused)) const char* const * params, StatType* file_info)
{
    if (get_argument_index_nouser() < 0)
    {
        return TRUE;
    }
    return (has_no_user(file_info) == 1);
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
 * \return boolean TRUE name filter matched or not given, FALSE no match found.
 */
static boolean filter_user(__attribute__((unused)) const char* path_to_examine, const char* const * params,
        __attribute__((unused))  StatType* file_info)
{
    if (get_argument_index_user() < 0)
    {
        return TRUE;
    }

    if (FALSE == user_exist(params[get_argument_index_user() + 1]))
    {
        cleanup(TRUE);
    }
    return TRUE;
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
 * \return boolean TRUE name filter matched or not given, FALSE no match found.
 */
static boolean filter_type(__attribute__((unused)) const char* path_to_examine, const char* const * params,
        StatType* file_info)
{
    const char* parameter1;

    if (get_argument_index_type() < 0)
    {
        return TRUE;
    }

    parameter1 = params[get_argument_index_type() + 1];
    /* check if option argument describes the same file type as file to examine has */
    return (*parameter1 == get_file_type(file_info));
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
static void print_result(const char* file_path, StatType* file_info)
{
    print_detail print_function[2];
    int outputs = 1;
    int i = 0;

    print_function[0] = print_detail_print;
    print_function[1] = print_detail_print;

    if (get_argument_index_ls() >= 0)
    {
        /* we have a -ls option */
        outputs = 2;
        if (get_argument_index_print() >= get_argument_index_ls())
        {
            /* -ls is given before -print or no -print given */
            print_function[0] = print_detail_ls;
        }
        else
        {
            print_function[1] = print_detail_ls;
        }
    }
    for (i = 0; i < outputs; ++i)
    {
        /* now print the output in the right order of option arguments */
        (print_function[i])(file_path, file_info);
    }

    return;
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

    if (!(file_info->st_mode & S_ISUID))
    {
        /*no UID-Bit */
        fprintf(stdout, "%c", (file_info->st_mode & S_IXUSR ? 'x' : '-'));
    }
    else if ((file_info->st_mode & S_ISUID) && (file_info->st_mode & S_IXUSR))
    {
        /*UID-Bit && Execute-Bit*/
        fprintf(stdout, "%c", (file_info->st_mode & S_ISUID ? 's' : '-'));
    }
    else
    {
        /*UID-Bit && !Execute-Bit*/
        fprintf(stdout, "%c", (file_info->st_mode & S_ISUID ? 'S' : '-'));
    }

    /* Print group permissions */
    fprintf(stdout, "%c", (file_info->st_mode & S_IRGRP ? 'r' : '-'));
    fprintf(stdout, "%c", (file_info->st_mode & S_IWGRP ? 'w' : '-'));

    if (!(file_info->st_mode & S_ISGID))
    {
        /*no GID-Bit */
        fprintf(stdout, "%c", (file_info->st_mode & S_IXGRP ? 'x' : '-'));
    }
    else if ((file_info->st_mode & S_ISGID) && (file_info->st_mode & S_IXGRP))
    {
        /*GID-Bit && Execute-Bit */
        fprintf(stdout, "%c", (file_info->st_mode & S_ISGID ? 's' : '-'));
    }
    else
    {
        /*GID-Bit && !Execute-Bit*/
        fprintf(stdout, "%c", (file_info->st_mode & S_ISGID ? 'S' : '-'));
    }

    /* Print other permissions */
    fprintf(stdout, "%c", (file_info->st_mode & S_IROTH ? 'r' : '-'));
    fprintf(stdout, "%c", (file_info->st_mode & S_IWOTH ? 'w' : '-'));

    if (!(file_info->st_mode & S_ISVTX))
    {
        /*Sticky-Bit*/
        fprintf(stdout, "%c", (file_info->st_mode & S_IXOTH ? 'x' : '-'));
    }
    else if ((file_info->st_mode & S_ISVTX) && (file_info->st_mode & S_IXOTH))
    {
        /*Sticky-Bit && Execute-Bit*/
        fprintf(stdout, "%c", (file_info->st_mode & S_ISVTX ? 't' : '-'));
    }
    else
    {
        /*Sticky-Bit && !Execute-Bit*/
        fprintf(stdout, "%c", (file_info->st_mode & S_ISVTX ? 'T' : '-'));
    }

    fprintf(stdout, "  ");
}

/**
 * \brief Print user name and group name to standard out.
 *
 * \param file_info with all file attributes read out from operating system.
 *
 * \return void
 **/
static void print_user_group(const StatType* file_info)
{
    struct passwd* password;
    struct group* group_info;

    /* Print user name */
    password = getpwuid(file_info->st_uid);
    if (NULL != password)
    {
        fprintf(stdout, "%5s", password->pw_name);
    }
    else
    {
        fprintf(stdout, "%5d", file_info->st_uid);
    }

    fprintf(stdout, " ");

    /* Print group name */
    group_info = getgrgid(file_info->st_gid);
    if (NULL != password)
    {
        fprintf(stdout, "%5s", group_info->gr_name);
    }
    else if (NULL != group_info)
    {
        fprintf(stdout, "%5s", group_info->gr_name);
    }
    else
    {
        fprintf(stdout, "%5d", file_info->st_gid);
    }
}

/**
 * \brief Print the detailed info of matched file to standard out.
 *
 * \param file_path Fully qualified file name with path read out from operating system.
 * \param file_info with all file attributes read out from operating system.
 *
 * \return void
 **/
static void print_detail_ls(const char* file_path, StatType* file_info)
{
    combine_ls(file_info);
    printf(" ");
    printf("%s\n", file_path);
}

/**
 * \brief Standard print, used by every match. Prints on standard out.
 *
 * \param file_path Fully qualified file name with path read out from operating system.
 * \param file_info with all file attributes read out from operating system.
 *
 * \return void
 **/
static void print_detail_print(const char* file_path, __attribute__((unused)) StatType* file_info)
{
    printf("%s\n", file_path);
}

/**
 * \brief Print on standard out the -ls arguments: number of i-nodes,blocks, permissions,
 number of links, owner, group, last modification time and directory name.
 symlinks.
 *
 * \param file_info with all file attributes read out from operating system.
 *
 * \return void
 **/
static void combine_ls(const StatType* file_info)
{
    /* Print i-node */
    fprintf(stdout, "%8lu    ", (unsigned long) file_info->st_ino);

    /* Print number of blocks */
    fprintf(stdout, "%3lu  ", (unsigned long) file_info->st_blocks / 2);

    print_file_permissions(file_info);

    /* Print number of hard links */
    fprintf(stdout, "%2lu ", (unsigned long) file_info->st_nlink);

    print_user_group(file_info);

    /* Print file size */
    fprintf(stdout, "%9lu  ", (unsigned long) file_info->st_size);

    print_file_change_time(file_info);
}
/*
 * =================================================================== eof ==
 */

