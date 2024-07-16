#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../core/include/socket_handler.h"
#include "../../core/include/queue_utils.h"
#include "../../core/include/utils.h"
#include "../include/collector.h"

// Funzione di confronto utilizzata dalla lista
long data_compare(void *a, void *b) {
    Data *val1 = (Data *) a;
    Data *val2 = (Data *) b;
    return(val1->result) - (val2->result);
}

static int insert_data(Queue ** dataQueue, char *buffer);
static void print_data_queue(Queue *dataQueue);
static void free_data_queue(Queue **dataQueue);

void start_collector() {    

    printf("aaaaaaaaaaaa\n");

    fflush(stdout);

    //eseguo il setup della socket per il Collector
    if(setup_socket_collector() == -1){


        perror("ERRORE: impossibile eseguire la setup_socket_collector() ");
        //rimuovo il file tmp della socket
        if(delete_tmp_socket() == -1){
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE);
    }


    // alloco un puntatore di tipo Queue
    Queue *dataQueue;
    // inizializzo la coda dove salvare i risultati e i nomi dei file
    create_queue(&dataQueue, data_compare);

    //alloco un puntatore per salvare il buffer 
    char *buffer = NULL;


    // Itero fino a il processo Master e' connesso in scrittura/ non vengono generati errrori
    while ((buffer = read_socket()) != NULL) {

        printf("patata\n");
        fflush(stdout);

        // controllo se read_socket() ha generato un errore
        if ((strncmp("ERRORE", buffer, strlen("ERRORE") + 1)) == 0) {

            
    
            perror("ERRORE: impossibile eseguire la read_socket() ");
            //libero memoria allocata su heap
            printf("patata1\n");
            free_data_queue(&dataQueue);
            //rimuovo il file tmp della socket
            printf("patata2\n");
            if(delete_tmp_socket() == -1){
                exit(EXIT_FAILURE);
            }
            //esco con fallimento
            printf("patata3\n");
            exit(EXIT_FAILURE);

        // controllo se il contenuto di buffer equivale a "STAMPA"
        } else if ((strncmp("STAMPA", buffer, strlen("STAMPA") + 1)) == 0) {
            // stampo gli i risultati e nomi dei file in modo ordinato fino ad ora ricevuti
            print_data_queue(dataQueue);

        } else {
            // inserisco nome del file e risultato nella coda, controllando errore
            if (insert_data(&dataQueue, buffer) == -1) {
                //insert_data() genera un errore
                perror("ERRORE: impossibile eseguire la close_socket_collector() ");
                //libero memoria allocata su heap per la coda di Data
                free_data_queue(&dataQueue);
                //libero memoria allocata su heap per il buffer letto dalla socket
                free(buffer);
                //rimuovo il file tmp della socket
                if(delete_tmp_socket() == -1){
                    exit(EXIT_FAILURE);
                }
                //esco con fallimento
                exit(EXIT_FAILURE);
            }

            // libero memoria allocata nell' heap per buffer
            free(buffer);
        }
    }

    //stampo tutti i risultati e nome dei file ricevuti in modo ordinato
    print_data_queue(dataQueue);
    
    //libero memori allocata nell' heap per dataQueue
    free_data_queue(&dataQueue);

    //chiudo la socket
    if(close_socket_collector() == -1){
        perror("ERRORE: impossibile eseguire la close_socket_collector() ");
        exit(EXIT_FAILURE);
    }
    
}

//Inserisce risultato e path relativo file in dataQueue in modo ordinato: 0 (successo), -1 (errore)
int insert_data(Queue ** dataQueue, char *buffer){

    //dichiaro un putnatore di tipo Data dove salavare il nuovo risultato/path relativo del file
    Data *newData;

    //alloco memoria su heap per newData    
    if((newData = (Data *) malloc(sizeof(Data))) == NULL){
        perror("ERRORE: impossibile eseguire la malloc() ");
        return -1;
    }

    //dichiaro size_t dove salvare lunghezza di buffer
    size_t length;

    //copio il risultato in newSata result
    memcpy(&(newData->result), buffer, sizeof(long));
    //copio lunghezza di buffer in length
    memcpy(&length, buffer + sizeof(long), sizeof(size_t));

    //alloco memoria su heap per path relativo file
    if((newData->fileName =(char *) malloc(sizeof(char) * length)) == NULL){
        perror("ERRORE: impossibile eseguire la malloc() ");
        return -1;
    }
    
    //copio path relativo file in newData->filename
    memcpy(newData->fileName, buffer + sizeof(long) + sizeof(size_t) , length);

    //inserisco newData in dataQueue in modo ordianto
    CHECK_FUN(queue_sorted_insert(dataQueue, newData), "ERRORE: impossibile eseguire la queue_sorted_insert() ");

    return 0;

}

//Libera memoria allocata su heap per dataQueue
void free_data_queue(Queue **dataQueue){

    //punatore per scorrere la coda
    Queue *tmp = *dataQueue;

    //itero fino a che non sono in fondo alla coda
    while(tmp != NULL){
        //salvo in data il value del nodo corrente della coda
        Data *data = tmp->value;
        //eseguo la free del path relativo del file contentuto nel nodo corrente
        free(data->fileName);
        //scorro la coda
        tmp = tmp->next; 
    }


    //eseguo la free della coda
    free_queue(dataQueue);
}


void print_data_queue(Queue *dataQueue){
    
    //itero fino a che non sono in fondo alla coda
    while(dataQueue != NULL){
        //salvo in data il value del nodo corrente della coda
        Data *data = dataQueue->value;
        //stampo risultato e path relativo del file contenuti nel nodo corrente
        printf("%ld %s\n", data->result, data->fileName);
        //scorro la coda
        dataQueue = dataQueue->next;

    }
}

