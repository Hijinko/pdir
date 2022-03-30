#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sysexits.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#define RWXRWXRX 0775
#define RWRWR 0664
#define DIRSEP "/"

static const char * get_args(int argc, char ** argv);
static int check_path_exists(const char * path);
static char * join_path(const char * base_dir, const char * path);
static int format_print(const char * action, const char * entity);
static void create_dirs(const char * base_dir, const char ** directories, 
                        size_t dir_size);
static void create_file(const char * base_dir, const char * filename);
static void copy_file(const char * src, const char * dest);
static char * get_exe_dir(void);

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
    const char * directories[] = {"bin", "src", "include", "doc", "test"};
    // create a the directores
    size_t dir_size = sizeof(directories) / sizeof(*directories);
    create_dirs(base_dir, directories, dir_size);
    // create a Makefile for the directory
    char * p_exe_dir = get_exe_dir();
    char * p_make_src = join_path(p_exe_dir, "Makefile");
    char * p_make_dest = join_path(base_dir, "Makefile");
    copy_file(p_make_src, p_make_dest);
    free(p_exe_dir);
    free(p_make_src);
    free(p_make_dest);
    p_exe_dir = NULL;
    p_make_src = NULL;
    p_make_dest = NULL;
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

/*
 * @brief prints a formatted message to stdout
 * @param action the what part of the sentence
 * @param entity the thing the action is happening to i.e. a file name
 * @return the number of characters printed
 */
static int format_print(const char * action, const char * entity)
{
    return printf("%-20s %s\n", action, entity);
}

/*
 * @brief creats directores listed in an array
 * @param base_dir the base directory to add directories too
 * @param directories an array of directories to create
 * @param dir_size the number of directories in the array directories
 */
void create_dirs(const char * base_dir, const char ** directories, size_t dir_size)
{
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
                format_print("Creating Directory", p_full_path);
                mkdir(p_full_path, RWXRWXRX);
                // add a gitignore to the directory so it is able to be pushed
                // to git
                create_file(p_full_path, ".gitignore");
            }
            else // the file in the directory already exits
            {
                format_print("Skipping Dirctory", p_full_path);
            }

            free(p_full_path);
            p_full_path = NULL;
        }

    }

}

/*
 * @brief creates a file at the given directory
 * @param base_dir the base directory to add directories too
 * @param filename the name of the file to create
 */
static void create_file(const char * base_dir, const char * filename)
{
    char * p_make_path = join_path(base_dir, filename);
    if (NULL != p_make_path)
    {

        if (-1 == check_path_exists(p_make_path))
        {
            format_print("Creating File", p_make_path);
            creat(p_make_path, RWRWR);
            free(p_make_path);
        }
        else
        {
            format_print("Skipping File", p_make_path);
        }

    }

}

/*
 * @brief copies the contents of one file to another
 * @param src the path to the source file
 * @param dest the path to the destination file
 */
static void copy_file(const char * src, const char * dest)
{
    FILE * source = fopen(src, "r");
    FILE * destination = fopen(dest, "w");

    if (0 == check_path_exists(src))
    {
        format_print("Creating File", dest);
        char ch = fgetc(source);
        while (EOF != ch)
        {
            fputc(ch, destination);
            ch = fgetc(source);
        }
        goto CLEAN_UP;
    }
    else {
        printf("path does not exist %s\n", src);
        printf("path does not exist %s\n", dest);
        goto CLEAN_UP;
    }

    return;
    CLEAN_UP:
    fclose(source);
    fclose(destination);
}

/*
 * @brief gets the directory of the pdir executable
 * @return the directory path in an allocated string
 */
static char * get_exe_dir(void)
{
    char * full_path = calloc(PATH_MAX, sizeof(*full_path));
    readlink("/proc/self/exe", full_path, PATH_MAX);
    size_t len = strlen(full_path);
    char * base_path = calloc(len, sizeof(*base_path));
    strncpy(base_path, full_path, len - strlen("pdir"));
    free(full_path);
    full_path = NULL;
    return base_path;
}