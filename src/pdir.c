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

static const char * get_args(int argc, char ** argv);
static int check_path_exists(const char * path);
static char * join_path(const char * base_dir, const char * path);

int main(int argc, char ** argv)
{
    // get the options for the program
    const char * base_dir = get_args(argc, argv);
    // cant run the program if there is no path set, this is for safty reasons
    if (NULL == base_dir) 
    {
        fprintf(stderr, "A valid file path is needed\n");
        return EX_USAGE;
    }

    // if the path does not exist then we must exit the program
    int path_err = check_path_exists(base_dir);

    if (-1 == path_err)
    {
        fprintf(stderr, "Path [%s] does not exist\n", base_dir);
        return EX_DATAERR;
    }

    // the direcoties that will be created
    const char * directories[] = {"bin", "src", "include", "test"};
    size_t dir_size = sizeof(directories) / sizeof(*directories);
    
    // create a the directores
    for(size_t index = 0; index < dir_size; index++)
    {
        char * p_full_path = join_path(base_dir, directories[index]);

        // cant add the path if p_full_path is NULL
        if (NULL != p_full_path)
        {
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
            p_full_path = NULL;
        }
    }
}

/*
 * @brief gets the filepath from the command line args
 * @param argc the number of arguments passed in from the command line
 * @param argv the commands passed in
 * @return the path to the directory to add to or NULL on error
 */
static const char * get_args(int argc, char ** argv)
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

/*
 * @brief checks if a file or directory path exits
 * @param path the path to check for existence
 * @return 0 if the path exists else -1
 */
static int check_path_exists(const char * path)
{
    struct stat path_buff;
    return stat(path, &path_buff);
}

static char * join_path(const char * base_dir, const char * path)
{
    // get the full path of the directory to create
    size_t base_len = strlen(base_dir);
    size_t path_len = strlen(path);
    size_t dirsep_len = strlen(DIRSEP);
    size_t path_size = base_len + path_len + dirsep_len + 1;
    char * p_full_path = calloc(path_size, sizeof(*p_full_path) + 1);
    
    if (NULL != p_full_path)
    {
        strncat(p_full_path, base_dir, base_len);

        // add a directory seperator character if needed for the path
        if (DIRSEP[0] != p_full_path[base_len - 1])
        {
            strncat(p_full_path, DIRSEP, dirsep_len);
        }

        strncat(p_full_path, path, path_len);
    }

    return p_full_path;
}