#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#define NUM_HASH 1000

char md5_hash[NUM_HASH][32];
char *passw[NUM_HASH];
char **dict;

pthread_cond_t found_condvar;
pthread_mutex_t passw_mutex;

int main(int argc, char* argv[])
{
    pthread_t prod_threads[6];
    pthread_t cons_thread;
    pthread_attr_t attr;

    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&passw_mutex, NULL);
    pthread_cond_init (&found_condvar, NULL);

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    
}