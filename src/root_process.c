#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../include/utils.h"

#define MAX_FILE_COUNT 10

#define WRITE (O_WRONLY | O_CREAT | O_TRUNC)
#define PERM (S_IRUSR | S_IWUSR)
char *output_file_folder = "output/final_submission/";

void getAbsolutePath(char *relPath, char *absPath)
{
    char cwd[PATH_MAX];

    getcwd(cwd, PATH_MAX);

    sprintf(absPath, "%s/%s", cwd, relPath);
}

void redirection(char **dup_list, char **retain_list, int size, char* root_dir){
    // TODO(overview): redirect standard output to an output file in output_file_folder("output/final_submission/")
    // (step1): determine the filename based on root_dir. e.g. if root_dir is "./root_directories/root1", the output file's name should be "root1.txt"

    char *rootDirExtracted = extract_filename(root_dir);
    char outFilePath[PATH_MAX];

    sprintf(outFilePath, "%s%s.txt", output_file_folder, rootDirExtracted);

    //(step2): redirect standard output to output file (output/final_submission/root*.txt)

    // Open output file and redirect stdout to it
    int outFile = open(outFilePath, WRITE, PERM);

    int stdoutSaved = dup(STDOUT_FILENO);
    dup2(outFile, STDOUT_FILENO);

    //(step3): read the content each symbolic link in dup_list, write the path as well as the content of symbolic link to output file(as shown in expected)
    for (int i = 0; i < size; i++)
    {
        printf("%s --> %s\n", dup_list[i], retain_list[i]);
    }

    // Flush stdout and restore original stdout
    fflush(stdout);
    close(outFile);
    dup2(stdoutSaved, STDOUT_FILENO);
}

void create_symlinks(char **dup_list, char **retain_list, int size) {
    //(): create symbolic link at the location of deleted duplicate file
    //(): dup_list[i] will be the symbolic link for retain_list[i]

    char fullPath[PATH_MAX];

    for (int i = 0; i < size; i++)
    {
        getAbsolutePath(retain_list[i], fullPath);
        symlink(fullPath, dup_list[i]);
    }
}

void delete_duplicate_files(char **dup_list, int size) {
    //(): delete duplicate files, each element in dup_list is the path of the duplicate file
    for(int i = 0; i < size; i++)
    {
        remove(dup_list[i]);
    }
}

// ./root_directories <directory>
int main(int argc, char* argv[]) {
    if (argc != 2) {
        // dir is the root_directories directory to start with
        // e.g. ./root_directories/root1
        printf("Usage: ./root <dir> \n");
        return 1;
    }

    //TODO(overview): fork the first non_leaf process associated with root directory("./root_directories/root*")

    char* root_directory = argv[1];
    char all_filepath_hashvalue[4098]; //buffer for gathering all data transferred from child process
    memset(all_filepath_hashvalue, 0, sizeof(all_filepath_hashvalue));// clean the buffer

    //(step1): construct pipe

    //(step2): fork() child process & read data from pipe to all_filepath_hashvalue

    int status;
    char **dup_list;
    char **retain_list;
    char pipeWriteEndStr[16];
    int rootPipe[2];

    pipe(rootPipe);
    
    sprintf(pipeWriteEndStr, "%i", rootPipe[1]);
    pid_t pid = fork();
    if (pid == 0)
    {

        // Child, exec a nonleaf process
        execl("./nonleaf_process", "nonleaf_process", root_directory, pipeWriteEndStr, NULL); 
        exit(0);
    }
    else if (pid < 0)
    {
        fprintf(stderr, "Failed to fork() in root process\n");
    }
    else
    {
        waitpid(pid, &status, 0);

        close(rootPipe[1]);

        // Parent, read data from pipe 
        readUntilFullOrEOF(rootPipe[0], all_filepath_hashvalue, 4098);


        //(step3): malloc dup_list and retain list & use parse_hash() in utils.c to parse all_filepath_hashvalue
        // dup_list: list of paths of duplicate files. We need to delete the files and create symbolic links at the location
        // retain_list: list of paths of unique files. We will create symbolic links for those files


        dup_list = malloc(sizeof(char*) * MAX_FILE_COUNT);
        retain_list = malloc(sizeof(char*) * MAX_FILE_COUNT);

        for (int i = 0; i < MAX_FILE_COUNT; i++)
        {
            dup_list[i] = NULL;
            retain_list[i] = NULL;
        }


        // Process string from child 
        int size = parse_hash(all_filepath_hashvalue, dup_list, retain_list); 
    
        //(step4): implement the functions
        delete_duplicate_files(dup_list,size);
        create_symlinks(dup_list, retain_list, size);
        redirection(dup_list, retain_list, size, argv[1]);

        //(step5): free any arrays that are allocated using malloc!!
        
        for (int i = 0; i < MAX_FILE_COUNT; i++)
        {
            free(dup_list[i]);
            free(retain_list[i]);

            dup_list[i] = NULL;
            retain_list[i] = NULL;
        }

        free(dup_list);
        free(retain_list);
    }
}
