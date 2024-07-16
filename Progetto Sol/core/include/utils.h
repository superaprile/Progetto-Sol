#ifndef UTILS_H
#define UTILS_H
#include <sys/types.h>

#define PATH_SIZE 255

#define PARSE_VALUE(str, var)                                                  \
    {                                                                          \
        long number;                                                           \
        if (isNumber(str, &number) == 0) {                                     \
            var = number;                                                      \
        } else {                                                               \
            fprintf(stderr, "ERRORE: '%s' non è un argomento valido \n", str); \
            return -1;                                                         \
        }                                                                      \
    }

// Se fun ha generato un errore, stampo messaggio di errore e restituisco -1
#define CHECK_FUN(fun, message) \
    {                          \
        if (fun == -1) {       \
            perror(message);   \
            return -1;         \
        }                      \
    }

#endif

int isNumber(const char *s, long *n);
int is_dat(char *fileName);
int check_file(char *fileName);
ssize_t readn(int fd, void *ptr, size_t n);
ssize_t writen(int fd, void *ptr, size_t n);
