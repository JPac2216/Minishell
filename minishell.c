/*******************************************************************************
 * Name        : minishell.c
 * Author      : Jake Paccione
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#define _GNU_SOURCE


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#define BLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"

volatile sig_atomic_t interrupted = 0;

/**
 * Changes the current working directory.
 */
void cd(int argc, char** argv, char* path){
    if (argc > 2){
        fprintf(stderr, "Error: Too many arguments to cd.\n");
        return;
    } else if(strcmp(argv[1], "~") == 0 || argc == 1){
        uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);

        if(pw == NULL){
            fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
            return;
            
        } else if (chdir(pw->pw_dir) != 0){
            fprintf(stderr, "Error: Cannot change directory to %s. %s.\n", pw->pw_dir, strerror(errno));
            return;
        }
    } else if (chdir(argv[1]) != 0){
        fprintf(stderr, "Error: Cannot change directory to %s. %s.\n", argv[1], strerror(errno));
        return;
    }
}

/**
 * Lists all files under current working directory.
 */
void lf(char* path){
    struct dirent* dirp;

    DIR* dp = opendir(path);
    if(dp == NULL){
        fprintf(stderr, "Error: Cannot open %s\n", path);
        return;
    }

    while((dirp = readdir(dp)) != NULL){
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0){
            continue;
        }
        printf("%s\n", dirp->d_name);
    }
    closedir(dp);
}

/**
 * Lists all the current processes in the system in the format <PID> <USER> <COMMAND> 
 */
void lp(){
    struct dirent* dirp;

    DIR* dp = opendir("/proc/");
    if(dp == NULL){
        fprintf(stderr, "Error: Cannot open /proc/.\n");
        return;
    }

    while((dirp = readdir(dp)) != NULL){
        if (atoi(dirp->d_name)){
            struct stat fileinfo;
            
            char dpath[PATH_MAX];
            snprintf(dpath, sizeof(dpath), "/proc/%s", dirp->d_name);

            if(stat(dpath, &fileinfo) == 0){
                uid_t owner = fileinfo.st_uid;
                struct passwd *pw = getpwuid(owner);

                if(pw == NULL){
                    fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
                    return;
                }

                FILE* stream;

                char fpath[PATH_MAX];
                snprintf(fpath, sizeof(fpath), "/proc/%s/cmdline", dirp->d_name);

                if ((stream = fopen(fpath, "r")) == NULL) {
                    perror("Error");
                    continue;
                }
                char cmd[PATH_MAX];
                size_t numBytes = fread(cmd, 1, sizeof(cmd)-1, stream);
                if (numBytes == 0){
                    fclose(stream);
                    continue;
                }
                cmd[numBytes] = 0;

                printf("%s %s %s\n", dirp->d_name, pw->pw_name, cmd);

                fclose(stream);
            } else {
                fprintf(stderr, "Error: Cannot get stat entry. %s\n", strerror(errno));
                return;
            }
            
        }
    }
    closedir(dp);
}

/**
 * Signal handler helper function
 */
void setTrue(int sig){
    wait(NULL);
    interrupted = 1;
    printf("\n");
}

/**
 * Main function that prints CWD, waits for and handles user input, controls necessary signals.
 */
int main(){
    // Signal handling
    struct sigaction action = {0};
    action.sa_handler = setTrue;
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) != 0){
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
    }

    while(1){
        // Getting CWD & formatting prompt
        char* pathname = getcwd(NULL, 0);
        if (pathname == NULL){
            fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
        }
        printf("%s[%s]%s> ", BLUE, pathname, DEFAULT);

        // Reading user input
        char* line = NULL;
        size_t len = 0;
        ssize_t nread;
        
        if (nread = getline(&line, &len, stdin) == -1){
            if (interrupted){
                clearerr(stdin);
                free(line);
                free(pathname);
                interrupted = 0;
                continue;
            }
            fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
        }

        len = strlen(line) - 1;
        line[len] = 0;

        // Initializing argc & argv
        int argc = 0;
        char** argv = malloc(sizeof(char*) * 9);
        for (int i = 0; i<9; i++){
            argv[i] = malloc(len + 1);
        }

        if(argv == NULL){
            fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
        }

        if (line[0] != 0){
            argc++;
            int j = 0;
            for(int i = 0; i < len; i++){
                if(line[i] != ' '){
                    argv[argc-1][j] = line[i];
                    j++;
                } else {
                    argv[argc-1][j] = 0;
                    j=0;
                    if (line[i+1] != 0){
                        argc++;
                    }
                }
            }
            argv[argc-1][j] = 0;
        } else {
            argv[0][0] = 0;
        }
        
        // Catching custom commands
        if (strcmp(argv[0], "cd") == 0){
            cd(argc, argv, pathname);
        } else if (strcmp(argv[0], "exit") == 0){
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[0], "pwd") == 0){
            printf("%s\n", pathname);
        } else if (strcmp(argv[0], "lf") == 0){
            lf(pathname);
        } else if (strcmp(argv[0], "lp") == 0){
            lp();
        } else {
            // Catching other commands
            pid_t pid = fork();
            if (pid < 0){
                fprintf(stderr, "Error: Cannot fork a process. %s\n", strerror(errno));
            }
            else if (pid == 0) {
                argv[argc] = NULL;
                execvp(argv[0], argv);
                fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                exit(EXIT_FAILURE);
            } else {
                if (wait(NULL) == -1){
                    if (errno != EINTR){
                        fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
                    }
                }
            }
        }

        //Freeing allocated space
        free(line);

        for (int i = 0; i<9; i++){
            free(argv[i]);
        }
        free(argv);

        free(pathname);
    }
}
