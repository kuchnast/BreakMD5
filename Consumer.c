#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <memory.h>
#include <sys/types.h>

FILE* open_file(const char *path)
{
    FILE* fp;

    fp = fopen(path, "r");

    if(!fp)
        fprintf(stderr, "Could not open %s\n", path);

    return fp;
}

int getHexVal(char c)
   {
       int rtVal = 0;

       if(c >= '0' && c <= '9')
       {
           rtVal = c - '0';
       }
       else
       {
           rtVal = c - 'a' + 10;
       }

       return rtVal;
   }


int read_dict(const char *path, char **dict)
{
    int fp;
    ssize_t nread;
    size_t len = 0;
    char *buf = NULL;
    char c;
    size_t nlines = 0;
    size_t i = 0;

    if((fp = open_file(path)) == NULL)
        return EXIT_FAILURE;

    while(1) // count lines
    {
        c = fgetc(fp);
        if(c == '\n')
            ++nlines;
        if(c == EOF)
            break;
    }

    fseek(fp, 0, SEEK_SET);
    dict = (char **)calloc(nlines, sizeof(char *));

    while ((nread = getline(&buf, &len, fp)) != -1)
    {
        if (nread == 1) continue;    // skip blank lines 
        
        if (buf[nread - 1] == '\n') // strip from '\n'
            buf[nread--] = '\0';

        dict[i] = (char *)calloc(++nread, sizeof(char));
        memcpy(dict[i++], buf, nread);
    }

    return EXIT_SUCCESS;
}

int read_md5(const char* path, char ** passw, size_t* size)
{
    FILE *fp;
    ssize_t nread;
    size_t len = 0;
    char *buf = NULL;
    size_t i = 0;

    if((fp = open_file(path)) == NULL)
        return EXIT_FAILURE;

    while ((nread = getline(&buf, &len, fp)) != -1 && i < size)
    {
        if (nread == 1) continue;    // skip blank lines 
        
        if (buf[nread - 1] == '\n') // strip from '\n'
            buf[nread--] = '\0';

        memcpy(passw[i++], buf, nread);

        
    }

    
}