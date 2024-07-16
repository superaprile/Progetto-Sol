#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

#include "../include/utils.h"

//Converte arg a long e lo salva in num :0 successo, -1 errore 
int isNumber(const char *arg, long *num) {
    
    //alloco un char * string e lo inizializzo a NULL
    char *string = NULL;

    //eseguo la strtlo di arg in base 10
    long valore = strtol(arg, &string, 10);

    //controllo se la strol ha generato errore under/over flow
    if (errno == ERANGE) {
        // in caso di errore restituisco -1
        perror("ERRORE: impossibile eseguire strtol ");
        return -1;
    }

    //controllo se la strtol ha avuto successo (nessun carattere invalido)
    if (string != NULL && *string == (char) 0) {
        //setto num al valore di ritorno di strtol
        *num = valore;
        //restituisco 0
        return 0;   
    }

    //in caso di caratteri invalidi restituisco -1
    return -1;
}

// Restituisce il tipo di fileName: 2 directory, 1 file regolare, 0 nessuno dei due e -1 in caso di errore
int check_file(char* fileName) {

    //alloco una struct stat per salvare info di fileName
    struct stat info;

    //salvo gli attiributi del file in info, controllando errore
    if(stat(fileName, &info) == -1){
        //controllo valore errore
        if(errno != ENOENT){
            //caso di errore diverso da ENOENT restituisco -1
            perror("ERRORE: impossibile eseguire la stat() ");
            return -1;
        }
        //altrimenti ignoro al fine di non terminare il programma se viene passato come argomento un file non esistente
    } 

    //controllo se il file si tratta di una file regolare
    if (S_ISREG(info.st_mode)) {
        return 1;
    }

    //controllo se il file si tratta di una Direcotory
    if (S_ISDIR(info.st_mode)) {
        return 2;
    }

    //se non sono in nessuno dei casi precedenti restituisco 0
    return 0;
}


//Restituisce 1 se fileName ha come estensione .dat, 0 altrimenti
int is_dat(char* fileName) {

    //alloco un char * e lo setto all' ultima occorrenza di '.' in fileName
    char* ext = strrchr(fileName, '.');
    
    //controllo se fileName ha come estensione .dat
    if (ext && ext != fileName && (strncmp(ext, ".dat", strlen(ext) + 1) == 0)) {
        //restituisco 1
        return 1;
    }

    // altrimenti restiuisco 0
    return 0;
}


//Legge n bytes da fd in ptr, restituendo numero bytes letti
ssize_t readn(int fd, void *ptr, size_t n) {

        
    
    //alloco un size_t nleft per salvare il numero di bytes rimanenti
    size_t nleft;
    //alloco un ssize_t nread per salvare il numero di bytes letti
    ssize_t nread;

    //setto nelft a n, ovvero al totale di bytes
    nleft = n;
    
    //itero fino a che ci sono bytes rimanenti
    while (nleft > 0) {
        
        //leggo nleft da fd in ptr, controllando errore
        if ((nread = read(fd, ptr, nleft)) == -1) {

            
            // se la read ha generato un errore, controllo nelft
            if (nleft == n) {
        
                // errore generato all prima lettura, restituisco -1
                perror("ERRORE: impossibile eseguire la read() ");
                return -1;
            } else {
                //errore genato al i-esima lettura, interrompo il ciclo e restituisco numero bytes letti 
          
                break;
            }
            //se ho letto 0 bytes vuol dire che sono a EOF, quindi interrompo il while
        } else if (nread == 0) {
            break; 
        }

        //aggiorno bytes rimanenti sottraendo quelli letti
        nleft -= nread;
        //aggiorno ptr alla prima posizione libera
        ptr = ((char *) ptr) + nread;
    }
    
    //restituisco numero di byte letti
    return (n - nleft); /* return >= 0 */
}

//Scrive n bytes da ptr in fd, restituendo numero bytes scritti
ssize_t writen(int fd, void *ptr, size_t n) {
    // alloco un size_t nleft per salvare il numero di bytes rimanenti
    size_t nleft;
    // alloco un ssize_t nread per salvare il numero di bytes scritti
    ssize_t nwritten;

    // setto nelft a n, ovvero al totale di bytes
    nleft = n;
    // itero finche' ci sono bytes rimanenti
    while (nleft > 0) {
        // scrivo nleft bytes da ptr in fd
        if ((nwritten = write(fd, ptr, nleft)) < 0) {
            // se la write ha generato un errore, controllo nelft
            if (nleft == n) {
                // errore generato all prima scrittura, restituisco -1
                perror("ERRORE: impossibile eseguire la write ");
                return -1;
            } else {
                //errore genato al i-esima scrittura, restituisco numero bytes scitti fino ad ora
                break;
            }
        // se ho scritto 0 bytes vuold dire che sono a EOF, quindi interrompo il while
        } else if (nwritten == 0) {
            break;
        }
        //aggiorno bytes rimanenti sottraendo quelli scritti
        nleft -= nwritten;
        //aggiorno ptr alla prima posizione libera
        ptr = ((char *) ptr) + nwritten;
    }
    //restituisco numero di byte scritti
    return (n - nleft); /* return >= 0 */
}