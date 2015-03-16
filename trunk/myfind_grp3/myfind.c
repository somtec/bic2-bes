/**
 * @file myfind.c
 *
 * @author Sebastian Brescanovic
 *
 *
 * @date 2015/03
 *
 */

/*
 * -------------------------------------------------------------- includes --
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <fnmatch.h>
#include <utmp.h>
#include <unistd.h>
#

/*
 * --------------------------------------------------------------- defines --
 */

/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
 */

static const char *const_PRM;

/*
 * ------------------------------------------------------------- functions --
 */

void do_file(const char *file_name, const int dparam, const char * const *parms);
void do_dir(const char *dir_name, const int dparam, const char * const *parms);
void printlink(char *name);
void strmode(mode_t mode, char *p);



int main(int argc, const char * const *argv) {
    int i_i = 0;
    long val = 0;
    const char *action;
    char *pointer_e;
    struct passwd *ppentry = NULL;
    
    errno = 0;
    
    const_PRM = argv[0];
    
    if(argc <= 1) {
        fprintf(stderr,"Usage: %s <directory> <action> ...\n", argv[0]);
        fprintf(stderr,"        -type [bcdpfls]\n");
        fprintf(stderr,"        -user <name/uid>\n");
        fprintf(stderr,"        -name <glob-pattern>\n");
        fprintf(stderr,"        -print\n");
        return EXIT_FAILURE;
    } else {
        /* Alle mitgegebenen Parameter auf korrektheit pr�fen */
        if(argc >= 3) {
            argv = &argv[2];
            
            for(i_i=2;i_i<argc;i_i++) {
                /* Diese Parameter sind erlaubt */
                
                if(strcmp(*argv,"-user") == 0 || strcmp(*argv,"-name") == 0 || strcmp(*argv,"-type") == 0 || strcmp(*argv,"-print")   == 0)
                   
                   {
                       
                       action = *argv;
                       
                       /* Diese Funktionen ben�tigen auch Argumente! */
                       if(strcmp(action,"-user") == 0 || strcmp(action,"-name")  == 0 ||  strcmp(action,"-type") == 0)
                          {
                              
                              if(i_i+2>argc) {
                                  fprintf(stderr,"%s: Missing argument to '%s'\n", const_PRM, action);
                                  return EXIT_FAILURE;
                              } else { argv++;
                                  i_i++;
                                  if(strcmp(action,"-type") == 0) {
                                      if(strlen(*argv) == 0 || strlen(*argv) > 1) {
                                          fprintf(stderr,"%s: Invalid argument '%s' to '%s'\n", const_PRM, *argv, action);
                                          exit(EXIT_FAILURE);
                                      } else {
                                          if(strcmp(*argv,"b") == 0 || strcmp(*argv,"c") == 0 || strcmp(*argv,"d") == 0 ||
                                             strcmp(*argv,"p") == 0 || strcmp(*argv,"f") == 0 || strcmp(*argv,"l") == 0 ||
                                             strcmp(*argv,"s") == 0) {
                                              /* Argument ist okay */
                                          } else {
                                              fprintf(stderr,"%s: Invalid argument '%s' to '%s'\n", const_PRM, *argv, action);
                                              return EXIT_FAILURE;
                                          }
                                      }
                                  }
                                  
                                  if(strcmp(action,"-user") == 0) {
                                      if((ppentry = getpwnam(*argv)) == NULL) {
                                          val = strtol(*argv, &pointer_e, 10);
                                          val = val;
                                          
                                          if (pointer_e == *argv) {
                                              fprintf(stderr,"%s: Invalid argument '%s' to '%s'\n", const_PRM, *argv, action);
                                              return EXIT_FAILURE;
                                          } else {
                                              if((ppentry = getpwuid(val)) == NULL) {
                                                  fprintf(stderr,"%s: Invalid argument '%s' to '%s'\n", const_PRM, *argv, action);
                                                  return EXIT_FAILURE;
                                              }
                                          }
                                      }
                                  }
                              }
                          }
                          } else {
                              fprintf(stderr,"%s: Ungültig %s\n", const_PRM, *argv);
                              return EXIT_FAILURE;
                          }
                          
                          argv++;
                          }
                          
                          argv = argv-argc;
                          }
                          do_dir(argv[1],argc,argv);
                          }
                          
                          return EXIT_SUCCESS;
                          }
                          
                          void strmode(mode_t mode, char *p) {
                              /* print type */
                              switch (mode & S_IFMT) {
                                  case S_IFDIR:
                                      *p++ = 'd';
                                      break;
                                  case S_IFLNK:
                                      *p++ = 'l';
                                      break;
                                  case S_IFCHR:
                                      *p++ = 'c';
                                      break;
                                  case S_IFBLK:
                                      *p++ = 'b';
                                      break;
                                  case S_IFREG:
                                      *p++ = '-';
                                      break;
                                  case S_IFSOCK:
                                      *p++ = 's';
                                  break;
                                #ifdef S_IFIFO
                                  case S_IFIFO:/* first in first out */
                                      *p++ = 'p';
                                      break;
                                        #endif
                                #ifdef S_IFWHT
                                  case S_IFWHT:
                                      *p++ = 'w';
                                      break;
                                    #endif
                                  default:
                                      *p++ = '?';
                                      break;
                              }
                              /* usr */
                              if (mode & S_IRUSR)
                                  *p++ = 'r';
                              else
                                  *p++ = '-';
                              if (mode & S_IWUSR)
                                  *p++ = 'w';
                              else
                                  *p++ = '-';
                              switch (mode & (S_IXUSR | S_ISUID)) {
                                  case 0:
                                      *p++ = '-';
                                      break;
                                  case S_IXUSR:
                                      *p++ = 'x';
                                      break;
                                  case S_ISUID:
                                      *p++ = 'S';
                                      break;
                                  case S_IXUSR | S_ISUID:
                                      *p++ = 's';
                                      break;
                              }
                              /* group */
                              if (mode & S_IRGRP)
                                  *p++ = 'r';
                              else
                                  *p++ = '-';
                              if (mode & S_IWGRP)
                                  *p++ = 'w';
                              else
                                  *p++ = '-';
                              switch (mode & (S_IXGRP | S_ISGID)) {
                                  case 0:
                                      *p++ = '-';
                                      break;
                                  case S_IXGRP:
                                      *p++ = 'x';
                                      break;
                                  case S_ISGID:
                                      *p++ = 'S';
                                      break;
                                  case S_IXGRP | S_ISGID:
                                      *p++ = 's';
                                      break;
                              }
                              /* other */
                              if (mode & S_IROTH)
                                  *p++ = 'r';
                              else
                                  *p++ = '-';
                              if (mode & S_IWOTH)
                                  *p++ = 'w';
                              else
                                  *p++ = '-';
                              switch (mode & (S_IXOTH | S_ISVTX)) {
                                  case 0:
                                      *p++ = '-';
                                      break;
                                  case S_IXOTH:
                                      *p++ = 'x';
                                      break;
                                  case S_ISVTX:
                                      *p++ = 'T';
                                      break;
                                  case S_IXOTH | S_ISVTX:
                                      *p++ = 't';
                                      break;
                              }
                              *p++ = ' ';
                              *p = '\0';
                          }
                          
                          void printlink(char *name)      {
                              int lnklen;
                              char path[PATH_MAX + 1];
                              
                              if ((lnklen = readlink(name, path, PATH_MAX)) == -1) {
                                  fprintf(stderr,"%s: linking fehler %s", const_PRM, name);
                                  return;
                              }
                              path[lnklen] = '\0';
                              fprintf(stdout," -> %s", path);
                          }
                          
                          
                          void do_file(const char *file_name, const int dparam, const char * const *parms) {
                              struct stat file_info;
                              struct passwd *ppentry = NULL;
                              struct group *pGrpEntry = NULL;
                              const char *argument;
                              const char *action;
                              char *indexof;
                              char *pointer_e;
                              char *name;
                              /*  char *test;*/
                              int i_i=0;
                              int i_len = 0;
                              /*  int i_test = 0;*/
                              int file_okay = 0;
                              int dir_ausgabe = 0;
                              long val = 0;
                           
                              
                              if(lstat(file_name, &file_info) != -1) {
                                  
                                  if(dparam >= 3) {
                                      parms = &parms[2];
                                      
                                      for(i_i=2;i_i<dparam;i_i++) {
                                          action = *parms;
                                          
                                          errno = 0;
                                          
                                          if(strcmp(action,"-nouser") == 0) {
                                              
                                              if((ppentry = getpwuid(file_info.st_uid)) != NULL)
                                                  file_okay = 1;
                                          }
                                          
                                          if(strcmp(action,"-nogroup") == 0) {
                                              
                                              if((pGrpEntry = getgrgid(file_info.st_uid)) != NULL)
                                                  file_okay = 1;
                                          }
                                          
                                          if(strcmp(action,"-print") == 0) {
                                              /*file_okay = 0;*/
                                              break;
                                          }
                                          
                                        
                                          
                                          
                                          if(strcmp(action,"-user") == 0 || strcmp(action,"-name")  == 0 ||
                                             strcmp(action,"-type") == 0) {
                                                 
                                                 parms++;
                                                 i_i++;
                                                 
                                                 argument = *parms;
                                                 
                                                 
                                                 errno = 0;
                                                 if(strcmp(action,"-type") == 0) {
                                                     dir_ausgabe = 1;
                                                     /* Regular file */
                                                     if(strcmp(argument,"f") == 0 && S_ISREG(file_info.st_mode) != 1 && S_ISDIR(file_info.st_mode) != 1)
                                                         file_okay = 1;
                                                     /* Directory */
                                                     if(strcmp(argument,"d") == 0) {
                                                         if(S_ISDIR(file_info.st_mode) != 1)
                                                             file_okay = 1;
                                                         else
                                                             dir_ausgabe = 0;
                                                     }
                                                     /* character device */
                                                     if(strcmp(argument,"c") == 0 && S_ISCHR(file_info.st_mode) != 1 && S_ISDIR(file_info.st_mode) != 1)
                                                         file_okay = 1;
                                                     /* block device */
                                                     if(strcmp(argument,"b") == 0 && S_ISBLK(file_info.st_mode) != 1 && S_ISDIR(file_info.st_mode) != 1)
                                                         file_okay = 1;
                                                     /* FIFO (named pipe) */
                                                     if(strcmp(argument,"p") == 0 && S_ISFIFO(file_info.st_mode) != 1 && S_ISDIR(file_info.st_mode) != 1)
                                                         file_okay = 1;
                                                     /* symbolic link */
                                                     if(strcmp(argument,"l") == 0 && S_ISLNK(file_info.st_mode) != 1 && S_ISDIR(file_info.st_mode) != 1)
                                                         file_okay = 1;
                                                     /* socket */
                                                     if(strcmp(argument,"s") == 0 && S_ISSOCK(file_info.st_mode) != 1 && S_ISDIR(file_info.st_mode) != 1)
                                                         file_okay = 1;
                                                 }
                                                 
                                                 if(strcmp(action,"-user") == 0) {
                                                     if((ppentry = getpwnam(argument)) != NULL) {
                                                         if(file_info.st_uid != ppentry->pw_uid)
                                                             file_okay = 1;
                                                     } else {
                                                         val = strtol(argument, &pointer_e, 10);
                                                         
                                                         if (pointer_e == argument) {
                                                             
                                                         } else {
                                                             if((ppentry = getpwuid(val)) != NULL)
                                                                 file_okay = 1;
                                                         }
                                                     }
                                                 }
                                                 
                                                 
                                                 if(strcmp(action,"-name") == 0) {
                                                     
                                                     indexof = strrchr(file_name, '/');
                                                     
                                                     if(indexof != NULL) {
                                                         i_len = indexof-file_name+1;
                                                         name = malloc(i_len * sizeof(char));
                                                         if(name != NULL)
                                                             strcpy(name, indexof+1);
                                                         else
                                                             fprintf(stderr,"%s: Memory owerflow: %s\n", const_PRM, file_name);
                                                     } else {
                                                         name = malloc(strlen(file_name) * sizeof(char));
                                                         if(name != NULL)
                                                             strcpy(name,file_name);
                                                         else
                                                             fprintf(stderr,"%s: Memory owerflow: %s\n", const_PRM, file_name);
                                                     }
                                                     
                                                     if(fnmatch(argument,name,FNM_PATHNAME) != 0)
                                                         file_okay = 1;
                                                     
                                                     free(name);
                                                     /*}*/
                                                 }
                                              
                                             }
                                             
                                             if(file_okay > 0)
                                             break;
                                             
                                             parms++;
                                             }
                                             
                                             parms = parms-i_i;
                                             }
                                             
                                             if(file_okay == 0) {
                                                 if(S_ISDIR(file_info.st_mode)) {
                                                     if(dir_ausgabe == 0) {
                                                         fprintf(stdout,"%s\n",file_name); /* Verzeichnisnamen ausgegeben */
                                                     }
                                                     do_dir(file_name,dparam,parms);
                                                 } else {
                                                    
                                             }
                                             } else {
                                                 
                                                 fprintf(stderr,"%s do_file Error: %d - %s", const_PRM, errno, strerror(errno));
                                             }
                                  
                              }
                              
                          }

                                             
                                             
                                             
                                             void do_dir(const char *dir_name, const int dparam, const char * const *parms) {
                                                 DIR *verzeichnis;
                                                 struct dirent *files;
                                                 char path[PATH_MAX];
                                                 int i_closedir = 0;
                                                 
                                                 verzeichnis = opendir(dir_name);
                                                 
                                                 if(verzeichnis != NULL) {
                                                     errno = 0;
                                                     while((files = readdir(verzeichnis)) != NULL) {
                                                         errno = 0;
                                                         snprintf(path, (size_t) PATH_MAX, "%s/%s", dir_name, files->d_name);
                                                         
                                                         if((strncmp(files->d_name, "..", 2) == 0  && strlen(files->d_name) == 2) ||
                                                            (strncmp(files->d_name, "." , 1) == 0  && strlen(files->d_name) == 1)) {
                                                         } else {
                                                             do_file(path,dparam,parms);
                                                         }
                                                     }
                                                     
                                                     i_closedir = closedir(verzeichnis);
                                                     if(i_closedir == -1) {
                                                         do_file(dir_name,dparam,parms);
                                                         
                                                     }
                                                 } else {
                                                     do_file(dir_name,dparam,parms);
                                                     
                                                 }
                                             }

                              
                              
                                          /*
                                           * =================================================================== eof ==
                                           */
                                              
                                              
                                              
                                              

