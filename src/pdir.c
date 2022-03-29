#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sysexits.h>
#include <string.h>
#include <sys/stat.h>
#define RWXRWXRX 0775
#define DIRSEP "/"

const char * get_args(int argc, char ** argv)
{
    int opt = getopt(argc, argv, "p:");
    const char * path = NULL;
    while(opt != -1)
    {
        switch(opt)
        {
            case 'p':
                path = optarg;
            break;
        }
        opt = getopt(argc, argv, ":p");
    }
    return path;
}

int check_path_exists(const char * path)
{
    struct stat path_buff;
    return stat(path, &path_buff);
}

int main(int argc, char ** argv)
{
    // get the options for the program
    const char * path = get_args(argc, argv);
    // cant run the program if there is no path set, this is for safty reasons
    if (NULL == path) 
    {
        fprintf(stderr, "A valid file path is needed\n");
        return EX_USAGE;
    }

    // if the path does not exist then we must exit the program
    int path_err = check_path_exists(path);

    if (-1 == path_err)
    {
        fprintf(stderr, "Path [%s] does not exist\n", path);
        return EX_DATAERR;
    }

    // the direcoties that will be created
    const char * directories[] = {"bin", "src", "include", "test"};
    size_t dir_size = sizeof(directories) / sizeof(*directories);

    for(size_t index = 0; index < dir_size; index++)
    {
        // get the full path of the directory to create
        size_t path_len = strlen(path);
        size_t dir_len = strlen(directories[index]);
        size_t dirsep_len = strlen(DIRSEP);
        size_t dir_path_size = path_len + dir_len + dir_len + 1;
        char * p_full_path = calloc(dir_path_size, sizeof(*p_full_path) + 1);
        strncat(p_full_path, path, path_len);
        // add a directory seperator character if needed for the path

        if (DIRSEP[0] != p_full_path[path_len - 1])
        {
            strncat(p_full_path, DIRSEP, dirsep_len);
        }

        strncat(p_full_path, directories[index], dir_len);
        // create the directory if it doesnt exits
        int err = check_path_exists(p_full_path);

        // the file does not exist so we can create it
        if (-1 == err)
        {
            printf("Creating Directory: %s\n", p_full_path);
            mkdir(p_full_path, RWXRWXRX);
        }
        else // the file in the directory already exits
        {
            printf("Skipping Directory: %s\n", p_full_path);
        }

        free(p_full_path);
    }
}