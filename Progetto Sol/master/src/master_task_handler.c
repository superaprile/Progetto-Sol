#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "../../core/include/pthread_utils.h"
#include "../../core/include/queue_utils.h"
#include "../include/master_config.h"
#include "../include/master_task_handler.h"

static Queue *conccurentQueue;
static int queueFlag = 1;
static pthread_mutex_t queue = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t flag = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t full = PTHREAD_COND_INITIALIZER;
static pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

static int conccurrent_queue_is_open();

//funzione di compare per la coda concorrente
static long mystrncmp(void *str1, void *str2){
    char *string1 = (char *) str1;
    char *string2 = (char *) str2;
    return (long) strncmp(string1, string2, strlen(string1) + 1);
}

//Inzializza la coda concorrente
void create_conccurent_queue(){
    create_queue(&conccurentQueue, mystrncmp);
}

//Inserisce una stringa sulla coda concorrente: 0 successo, -1 errore
int push_task(char *fileName) {
    
    //prendo la lock sulla coda
    LOCK(&queue);

    //ittero finche' la coda risulta piena
    while (len_queue(conccurentQueue) == CONCURRENT_QUEUE_SIZE) {
        //attendo che venga fatta la pop di un task rilasciando la lock
        WAIT(&full, &queue);
    }

    char *task;

    //alloco memoria sull'heap per salvare nome del file passato
    if((task = malloc(sizeof(char) * strlen(fileName) + 1)) == NULL){
        perror("ERRORE: impossibile eseguire la malloc ");
        return -1;
    }

    //copio la stringa nella memoria allocata sullo heap puntata da task
    strncpy(task, fileName, strlen(fileName) + 1);

    //inserico il task nella coda conccorrente
    if(queue_push(&conccurentQueue, task) == -1){
        perror("ERRORE: impossibile eseguire la queue_push() ");
        //libero memoria allocata sull' heap per il task
        free(task);
        return -1;
    }

    //segnalo l'inserimento del task nella coda concorrente ai worker in attesa
    SIGNAL(&empty);

    //rilascio la lock sulla coda concorrente
    UNLOCK(&queue);

    return 0;
}

// Restituisce il primo task nella coda concorrente
char *pop_task() {

    //prendo la lock sulla coda concorrente
    LOCK(&queue);

    //itero finchè la coda risulta vuota ed è aperta(il dispatcher sta continuando a inserire i task)
    while (len_queue(conccurentQueue) == 0 && conccurrent_queue_is_open()) {
        //attendo che venga fatta la push di un task rilasciando la lock
        WAIT(&empty, &queue);
    }

    //controllo se la coda risulta vuota e la coda risulta chiusa (caso di terminazione del worker)
    if(len_queue(conccurentQueue) == 0 && !conccurrent_queue_is_open()){
        //rilascio la lock sulla coda concorrente
        UNLOCK(&queue);
        //restituisco NULL 
        return NULL;
    }

    //rimuovo il primo task della coda concorrente
    char *name = queue_pop(&conccurentQueue);

    //segnalo al dispatcher che la coda e' stato rimosso un task dalla coda
    SIGNAL(&full);

    //rilascio la lock sulla coda concorrente
    UNLOCK(&queue);

    //restituisco il task
    return name;
}

// Setta il flag della coda concorrente a 0(chiusa)
void close_conccurrent_queue(){
    //prendo la lock sul flag della coda concorrente
    LOCK(&flag);
    //setto il flag della coda concorrente a 0 (chiusa)
    queueFlag = 0;
    //rilascio la lock sul flag della coda concorrente
    UNLOCK(&flag);
    //prendo la lock sulla coda concorrente
    LOCK(&queue);
    //segnalo ai thread worker in attesa la terminazione del produttore
    BCAST(&empty);
    //rilascio la lock sulla coda concorrente
    UNLOCK(&queue);
    //rilascio la lock sul flag della coda concorrente

}

// Controlla lo stato della coda, restituendo: 1 (aperta), 0 (chiusa)
int conccurrent_queue_is_open(){
    //prendo la lock sul flag della coda concorrente
    LOCK(&flag);
    //salvo il valore del flag 
    int flagResponse = queueFlag;
    //rilascio la lock sul flag della coda concorrente
    UNLOCK(&flag);
    //restituisco il valore del flag della coda concorrente
    return flagResponse;

}
