#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/**
 * Reads from a file descriptor for a specified number of bytes or until an EOF,
 * whichever comes first.
 *
 * @author Alejandro Zeise
 */
int readUntilFullOrEOF(int fd, char *buffer, int bytesToRead);

//used in leaf_process.c for inter submission
char* extract_filename(char* path);
char* extract_root_directory(const char* path);

//used in root_process.c for final submission
int parse_hash(char * file_hashes, char**dup_list, char** retain_list);
void sanitize_dup_retain(char **dup_list, char **retain_list, int size);
void remove_filepath_duplicate(char **dup_list, char **retain_list, int *size);
