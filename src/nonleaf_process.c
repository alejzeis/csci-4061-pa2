#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define BUFFER_SIZE       1024
#define MAX_CHILD_PROCESSES 20

int readUntilFullOrEOF(int fd, char *buffer, int bytesToRead);
void traverseDir(DIR *dir, char *dirPath, int pipeToParent);

int readUntilFullOrEOF(int fd, char *buffer, int bytesToRead)
{
    int tempPos;
    int pos = 0;
    while ((tempPos = read(fd, buffer, bytesToRead)) > 0)
    {
        pos += tempPos;
    }

    return pos;
}

void traverseDir(DIR *dir, char *dirPath, int pipeToParent)
{
    struct dirent *direntry;
    int waitStatus;
    int len;
    int processCount = 0;
    char pipeFdStr[4];
    char direntPath[BUFFER_SIZE];
    char pipeReadBuffers[MAX_CHILD_PROCESSES][BUFFER_SIZE];
    int pipes[MAX_CHILD_PROCESSES][2];
    pid_t pids[MAX_CHILD_PROCESSES];
    
    while((direntry = readdir(dir)) != NULL)
    {
        if (direntry->d_type == DT_REG || direntry->d_type == DT_DIR)
        {
            pipe(pipes[processCount++]);

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
                    execl("./leaf_process", direntPath, pipeFdStr, NULL);
                }
                else
                {
                    execl("./nonleaf_process", direntPath, pipeFdStr, NULL);
                }
            }
            else if (pids[processCount] < 0)
            {
                fprintf(stderr, "Failed to fork() for direntry: %s\n", direntry->d_name);
            }
            else
            {
                // Parent
                
                close(pipes[processCount][1]); // Close write end of the current pipe
            }
        }
    }

    // Read from each open pipe, send the data to the parent
    for (unsigned int i = 0; i < processCount; i++)
    {
        len = readUntilFullOrEOF(pipes[i][0], pipeReadBuffers[i], BUFFER_SIZE);
                   
        write(pipeToParent, pipeReadBuffers[i], len);
    }
    

    // Wait for all children to quit
    for (unsigned int i = 0; i < processCount; i++)
    {
        waitpid(pids[i], &waitStatus, 0);
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

    return 0;
}
