#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include "../../core/include/pthread_utils.h"
#include "../../core/include/signal_handler.h"
#include "../include/master_task_producer.h"
#include "../include/master_task_handler.h"
#include "../include/master_task_worker.h"
#include "../include/master_config.h"
#include "../include/master_farm.h"

static int create_thread_pool(pthread_t threadPool[]);
static void wait_thread_pool(pthread_t threadPool[]);

// Inzializza la coda concorrente, crea la threadpool, esegue il setup dei signal handler dei segnali, sblocca i segnali, processa i file binari e aspetta la terminazione dei thread worker: 0 succeso, -1 errore
int create_farm(char * fileList[], int size){

    //inzializzo la coda concorrente
    create_conccurent_queue();
    
    //alloco un array di thread
    pthread_t threadPool[THREAD_WORKERS_AMOUNT]; 

    //creo i thread worker
    if(create_thread_pool(threadPool) == -1){
        //in caso di errore chiudo la coda concorrente al fine di terminare i thread worker creati
        close_conccurrent_queue();
        return -1;
    }

    //setto la gestione dei segnali SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE
    if(setup_signal_handler() == -1){
        //in caso di errore chiudo la coda concorrente al fine di terminare i thread worker creati
        close_conccurrent_queue();
        return -1;
    }

    //sblocco i segnali SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE
    if(unblock_signal() == -1){
        //in caso di errore chiudo la coda concorrente al fine di terminare i thread worker creati
        close_conccurrent_queue();
        return -1;
    }

    //eseguo la push dei path relativi dei file binari sulla coda concorrente
    if(produce_tasks(fileList, size) == -1){
        //in caso di errore/segnal di terminazione chiudo la coda concorrente al fine di terminare i thread worker creati
        close_conccurrent_queue();
        return -1;
    }
    
    //aspetto la terminazione della threadPool
    wait_thread_pool(threadPool);

    return 0;

}

// Crea la threadpool: 0 successo, -1 errore
int create_thread_pool(pthread_t threadPool[]) {

    //Itero da 0 fino a THREAD_WORKERS_AMOUNT
    for (size_t i = 0; i < THREAD_WORKERS_AMOUNT; i++) {

        //creo l' i-esimo thread worker
        if(pthread_create(&threadPool[i], NULL, handle_task, NULL) != 0){
            //in caso di errore restituisco -1
            perror("ERRORE: impossibile eseguire la pthread_create() ");
            return -1;
        }
    }

    return 0;
}

// Esegue la join su tutti i thread worker: 0 successo, -1 errore
void wait_thread_pool(pthread_t threadPool[]) {

    //Itero tutti i thread worker
    for (size_t i = 0; i < THREAD_WORKERS_AMOUNT; i++) {

        //eseguo la join sull' i-esimo thread worker
        JOIN(threadPool[i]);
    }

}
