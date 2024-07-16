#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "../include/pthread_utils.h"
#include "../include/utils.h"
#include "../include/socket_handler.h"

#define UNIX_PATH_MAX 108
#define SOCKNAME "farm.sck"

static int fd_skt = -1;
static int fd_c = -1;
static struct sockaddr_un sa;
static pthread_mutex_t sck = PTHREAD_MUTEX_INITIALIZER;


// Creo la socket: 0 successo, -1 errore 
static int create_socket() {
    //copio in sa.sun_path il nome della socket
    strncpy(sa.sun_path, SOCKNAME, UNIX_PATH_MAX - 1);

    //setto il tipo di socket a AF_UNIX
    sa.sun_family = AF_UNIX;

    //creo la socket
    CHECK_FUN((fd_skt = socket(AF_UNIX, SOCK_STREAM, 0)), "ERRORE: impossibile creare la socket ");

    return 0;
}

// Esegue il setup della socket per il processo in ascolto: 0 successo, -1 errore 
int setup_socket_collector() {

    // creo la socket
    CHECK_FUN(create_socket(), "ERRORE: impossibile eseguire la create_socket() ");
    // eseguo il bind della socket
    CHECK_FUN(bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa)), "ERRORE: impossibile eseguire la bind() ");

    // // mi metto in ascolto sulla socket
    // CHECK_FUN(listen(fd_skt, SOMAXCONN), "ERRORE: impossibile eseguire la listen() ");

    // // accetto la connessione con il Master
    // CHECK_FUN((fd_c = accept(fd_skt, NULL, 0)), "ERRORE: impossibile eseguire la accept() ");

    return 0;
}

// Esegue il setup della socket per il processo "client": 0 successo, -1 errore 
int setup_socket_master() {

    //setto il numero di tentavi da riprovare in caso di conncection refused/socket non creata a 5;
    int retry = 5;
    //creo la socket
    CHECK_FUN(create_socket(), "ERRORE: impossibile eseguire la create_socket() ");
    // itero fino a che non riesco a connettermi alla socket
    while (connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        //se tentativi > 0 e socket non esiste oppure connessione rifiutata , sleep 1 secondo e riprovo
        if ((errno == ENOENT  || errno == ECONNREFUSED) && retry > 0){
            retry--;
            sleep(1);
        } else {
            perror("ERRORE: impossibile eseguire la connect() ");
            return -1;
        }       
    }

    return 0;
}


// Effettua la scrittura di bufferSize sulla socket, seguita dalla scrittura del buffer: 0 successo, -1 errore
int write_socket(char *buffer, size_t bufferSize) {

    //prendo la lock sulla socket
    LOCK(&sck);
    
    //scrivo buffersize sulla socket
    if(writen(fd_skt, &bufferSize, sizeof(size_t)) != sizeof(size_t)){
        // se non sono riuscito a scrivere tutto i bytes restituisco -1
        perror("ERRORE: impossibile eseguire la writen() di bufferSize ");
        return -1;
    }

    //scrivo buffer sulla socket
    if(writen(fd_skt, buffer, bufferSize) != bufferSize){
        // se non sono riuscito a scrivere tutto i bytes restituisco -1
        perror("ERRORE: impossibile eseguire la writen() di buffer ");
        return -1;
    }

    //rilascio la lock sulla socket
    UNLOCK(&sck);

    return 0;
}

// Effettua la lettura di bufferSize dalla socket, seguita dalla lettura del buffer. Resituisce: buffer (successo), NULL (se non ci sono piu' scrittori), "ERRRORE" in caso di errore
char *read_socket() {

    
    //alloco un size_t per la lunghezza del buffer
    size_t bufferSize;

    //alloco ssize_t per salvare numero di bytes letti
    ssize_t nread;

    fflush(stdout);
    //leggo bufferSize dalla socket, controllando errore
    if((nread = readn(fd_c, &bufferSize, sizeof(size_t))) != sizeof(size_t)){
        //controllo non ci sono piu' scrittori connessi
        if(nread == 0){
            //restiusco NULL
            return NULL;
        } else {
            //altrimenti restituisco "ERRORE"
            return "ERRORE";
        }

    }

    
        

    //allloco buffer sull' heap dove salvare buffer presente sulla socket
    char *buffer;
    if((buffer =malloc(sizeof(char) * bufferSize)) == NULL){
        perror("ERRORE: impossibile eseguire la malloc() ");
        return NULL;
    }

    //leggo il buffer dalla socket, controllando errore
    if((nread = readn(fd_c, buffer, bufferSize)) != bufferSize){
        //libero memoria allocata per buffer nell'heap
        free(buffer);
        //controllo non ci sono piu' scrittori connessi
        if(nread == 0){
            //restiusco NULL
            return NULL;
        } else {
            //altrimenti restituisco "ERRORE"
            return "ERRORE";
        }

    }

    //restituisco buffer
    return buffer;
}

// Chiude la socket lato Master, restituendo: 0 successo, -1 errore
int close_socket_master() {

    CHECK_FUN(close(fd_skt), "ERRORE: impossibile eseguire la close()");

    return 0;
}

int delete_tmp_socket(){
    CHECK_FUN(unlink(SOCKNAME), "ERRORE: impossibile eseguire l'unlink() ");

    return 0;
}

// Chiude la socket lato Collector, restituendo: 0 successo, -1 errore
int close_socket_collector() {

    CHECK_FUN(close(fd_skt), "ERRORE: impossibile eseguire la close()");
    CHECK_FUN(close(fd_c), "ERRORE: impossibile eseguire la close() ");
    CHECK_FUN(delete_tmp_socket(), "ERRORE: impossibile eseguire la delete_tmp_socket() ");

    return 0;
}




