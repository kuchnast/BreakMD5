#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>


char** read_file(const char *path)
{
    int fd;
    struct stat statbuf;
    char **buf;

    fd = open(path, O_RDONLY);

    if(fd < 0)
    {
        printf("Could not open %s\n", path);
        return NULL;
    }

    
    if(fstat(fd, &statbuf))
    {
        printf("Could not open %s\n", path);
        return NULL;
    }

}
