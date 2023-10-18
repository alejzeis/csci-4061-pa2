#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/hash.h"
#include "../include/utils.h"
#include <stdbool.h>

#define HASH_SIZE (SHA256_BLOCK_SIZE * 2 + 1)

static const char *output_file_folder = "output/inter_submission/";

void doIntermediateSubmission(char *filePath, char *strToParent, bool attemptMkdirIfFail);

void doIntermediateSubmission(char *filePath, char *strToParent, bool attemptMkdirIfFail)
{
    FILE *outputFile;
    char *fileName;
    char *rootDirectory;
    char constructedFilePath[PATH_MAX];
    char mkdirPath[PATH_MAX];

    fileName = extract_filename(filePath);
    rootDirectory = extract_root_directory(filePath);

    // Construct path to output file
    sprintf(constructedFilePath, "%s/%s/%s", output_file_folder, rootDirectory, fileName);

    outputFile = fopen(constructedFilePath, "w");
    if (NULL == outputFile && attemptMkdirIfFail)
    {
        // Output Directories probably do not exist, attempt to create it
        
        sprintf(mkdirPath, "%s%s", output_file_folder, rootDirectory);
        mkdir("output", 0770);
        mkdir("output/inter_submission", 0770);
        mkdir(mkdirPath, 0770);

        // Retry to create the output file
        doIntermediateSubmission(filePath, strToParent, false);
    }
    else if(NULL == outputFile)
    {
        fprintf(stderr, "Failed to open output file: %s\n", constructedFilePath);
    }
    else
    {
        fprintf(outputFile, "%s", strToParent);

        fclose(outputFile);
    }

    // Free root directory string, allocated in extract_root_directory()
    free(rootDirectory);
}

int main(int argc, char* argv[]) 
{
    char hash[HASH_SIZE];

    char strToParent[BUFFER_SIZE];
    char *filePath;
    int pipeWriteEnd;

    memset(hash, 0, HASH_SIZE);

    if (argc != 3) 
    {
        printf("Usage: Inter Submission --> ./leaf_process <file_path> 0\n");
        printf("Usage: Final Submission --> ./leaf_process <file_path> <pipe_write_end>\n");
        return -1;
    }
    //(): get <file_path> <pipe_write_end> from argv[]
    filePath = argv[1];
    pipeWriteEnd = atoi(argv[2]);


    //(): create the hash of given file
    hash_data_block(hash, filePath);


    //(): construct string write to pipe. The format is "<file_path>|<hash_value>"
    sprintf(strToParent, "%s|%s|", filePath, hash);

    if (1) 
    {
        //(inter submission)
        //(overview): create a file in output_file_folder("output/inter_submission/root*") and write the constructed string to the file
        //(step1): extract the file_name from file_path using extract_filename() in utils.c
        //(step2): extract the root directory(e.g. root1 or root2 or root3) from file_path using extract_root_directory() in utils.c
        //(step3): get the location of the new file (e.g. "output/inter_submission/root1" or "output/inter_submission/root2" or "output/inter_submission/root3")
        //(step4): create and write to file, and then close file
        //(step5): free any arrays that are allocated using malloc!! Free the string returned from extract_root_directory()!! It is allocated using malloc in extract_root_directory()
    
        doIntermediateSubmission(filePath, strToParent, true);
    }
    else
    {
        //TODO(final submission): write the string to pipe

        exit(0);

    }

    exit(0);
}
