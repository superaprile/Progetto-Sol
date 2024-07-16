#ifndef MASTER_CONFIG_H
#define MASTER_CONFIG_H

extern size_t THREAD_WORKERS_AMOUNT;
extern size_t CONCURRENT_QUEUE_SIZE;
extern char * DIR_NAME;
extern size_t DELAY;
extern int FIRST_FILE_INDEX;

int read_arguments(int argc, char *argv[]);

#endif
