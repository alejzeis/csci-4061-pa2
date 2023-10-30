#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#include "../include/utils.h"

#define BUFFER_SIZE       1024
#define MAX_CHILD_PROCESSES 20

void traverseDir(DIR *dir, char *dirPath, int pipeToParent);

void traverseDir(DIR *dir, char *dirPath, int pipeToParent)
{
    struct dirent *direntry;
    int waitStatus;
    int len;
    int processCount = 0;
    char pipeFdStr[9];
    char direntPath[BUFFER_SIZE];
    char pipeReadBuffers[MAX_CHILD_PROCESSES][BUFFER_SIZE];
    int pipes[MAX_CHILD_PROCESSES][2];
    pid_t pids[MAX_CHILD_PROCESSES];
    
    while((direntry = readdir(dir)) != NULL)
    {
        if ((direntry->d_type == DT_REG || direntry->d_type == DT_DIR) && strcmp(direntry->d_name, ".") != 0 && strcmp(direntry->d_name, "..") != 0)
        {
            int a[2];
            pipe(a);
            pipes[processCount][0] = a[0];
            pipes[processCount][1] = a[1];

            // Convert write-end of pipe FD to string for use in exec() call
            sprintf(pipeFdStr, "%i", pipes[processCount][1]);
            // Construct full path to the directory entry
            sprintf(direntPath, "%s/%s", dirPath, direntry->d_name);

            // Fork process, add pid to our array
            pids[processCount] = fork();

            if (pids[processCount] == 0)
            {
                // Child
                
                close(pipes[processCount][0]); // Close read end of the current pipe

                // Execute nonleaf or leaf process depending on if the entry is a file or directory
                if (direntry->d_type == DT_REG)
                {
                    execl("./leaf_process", "leaf_process", direntPath, pipeFdStr, NULL);
                    exit(0);
                }
                else
                {
                    execl("./nonleaf_process", "nonleaf_process", direntPath, pipeFdStr, NULL);
                    exit(0);
                }
            }
            else if (pids[processCount] < 0)
            {
                fprintf(stderr, "Failed to fork() for direntry: %s\n", direntry->d_name);
                break;
            }
            else
            {
                // Parent
                close(pipes[processCount][1]); // Close write end of the current pipe
            }

            processCount++;
        }
    }

    // Wait for all children to quit
    for (unsigned int i = 0; i < processCount; i++)
    {
        waitpid(pids[i], &waitStatus, 0);
    }

    // Read from each open pipe, send the data to the parent
    for (unsigned int i = 0; i < processCount; i++)
    {
        len = readUntilFullOrEOF(pipes[i][0], pipeReadBuffers[i], BUFFER_SIZE);

        write(pipeToParent, pipeReadBuffers[i], len);
    }
}

int main(int argc, char* argv[]) {
    char *dirPath;
    int pipeToParent;
    DIR *dir;

    if (argc != 3) {
        printf("Usage: ./nonleaf_process <directory_path> <pipe_write_end> \n");
        return 1;
    }
    //(overview): fork the child processes(non-leaf process or leaf process) each associated with items under <directory_path>

    //(step1): get <file_path> <pipe_write_end> from argv[]
    dirPath = argv[1];
    pipeToParent = atoi(argv[2]);


    //(step3): open directory
    dir = opendir(dirPath);
    if (NULL == dir)
    {
        fprintf(stderr, "Failed to open directory: %s\n", dirPath);
    }
    else
    {
        traverseDir(dir, dirPath, pipeToParent);
    }

    close(pipeToParent);
    //printf("nonleaf exit [closed pipe %i] %s\n", pipeToParent, dirPath);
    
    return 0;
}
