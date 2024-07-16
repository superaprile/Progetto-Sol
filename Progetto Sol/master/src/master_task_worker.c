#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "../../core/include/socket_handler.h"
#include "../include/master_task_handler.h"
#include "../include/master_task_worker.h"

static long read_dat(char *fileName); 
static int send_result(long result, char *fileName);

void *handle_task(void *arg) {

    //alloco un puntatore dove salvare path relativo del file binario
    char *fileName;

    //Itero fino a che ci sono task e la coda concorrente e' aperta
    while ((fileName = pop_task()) != NULL) {

        //alloco long per il risultato
        long result;

        //salvo il valore di ritorno di read_dat in result, controllando il caso di errore
        if((result = read_dat(fileName)) == -1){
            perror("ERRORE: impossibile eseguire la read_dat() ");
            //libero la memoria allocata nell'heap per il path relativo del file binario
            free(fileName);
            // termino il thread worker
            pthread_exit(NULL);

        }

        //invio tramite socket il risultato e il path relativo del file al Collector, controllando il caso di errore
        if(send_result(result, fileName) == -1){
            perror("ERRORE: impossibile eseguire la send_result() ");
            // libero la memoria nell'heap del nome del file binario
            free(fileName);
            // termino il thread worker
            pthread_exit(NULL);

        }

        // libero la memoria nell'heap del nome del file binario
        free(fileName);
    }

    return (void *)0;
}

// Processa il file binario restituendo: risultato (successo), -1 errore
long read_dat(char *fileName) {
    
    //alloco puntatore per il file binario
    FILE *file = NULL;

    long n;
    long tot = 0;
    int i = 0;

    //eseguo la open del file binario in lettura
    if ((file = fopen(fileName, "rb")) == NULL) {
        perror("ERRORE: impossibile eseguire la fopen ");
        return -1;
    }

    //leggo un long alla volta fino a end of file
    while (fread(&n, sizeof(long), 1, file) != 0) {

        //aggiorno tot
        tot = tot + (i * n);
        //icremento i
        i++;
    }

    //controllo se la fread ha generato un errore
    if(!feof(file)){
        perror("ERRORE: impossibile eseguire la fread() completamente ");
        //eseguo la close del file binario
        if (fclose(file) != 0) {
            // se la close genera un errore eseguo un exit con fallimento
            perror("ERRORE: impossibile eseguire la fclose() ");
            return -1;
        }
        return -1;
    }

    //eseguo la close del file binario
    if (fclose(file) != 0) {
        // se la close genera un errore eseguo un exit con fallimento
        perror("ERRORE: impossibile eseguire la fclose() ");
        return -1;
    }

    //restituisco il risultato
    return tot;
}

//Scive il nome del file e il risultato sulla socket per inviarli a Collector: 0 successo, -1 errore
int send_result(long result, char *fileName) {
    
    //salvo la lunghezza del path relativo del file binario in lenght
    size_t length = strlen(fileName) + 1;

    //salvo la lunghezza del buffer da scrivere sulla socket in size
    int size = sizeof(long) + sizeof(size_t) + length;
    
    //alloca un buffer da scrivere sulla socket grande size
    char buffer[size];

    //copio sul buffer il risultato
    memcpy(buffer, &result, sizeof(long));
    //aggiorno il buffer alla prima posizione libera e copio la lunghezza del path relativo del file binario
    memcpy(buffer + sizeof(long), &length, sizeof(size_t));
    //aggiorno nuovamente il buffer alla prima posizione libera e copio il path relativo del binario
    memcpy(buffer + sizeof(long) + sizeof(size_t), fileName, length);

    //scrive il buffer sulla socket
    if(write_socket(buffer, size) == -1){
        perror("ERRORE: impossibile eseguire la write_socket() ");
        return -1;
    }

    //restituisco 0
    return 0;

}