#include <stdio.h>
#include <stdlib.h>

#include "../include/utils.h"
#include "../include/queue_utils.h"

static long (*fun_compare) (void *, void *);

// Inizializza la coda a NULL e setta la funzione di confronto
void create_queue(Queue **queue, long (*compare) (void *, void *)){
    *queue = NULL;
    fun_compare = compare;
}

// Inserisce elemento in ordine crescente nella coda: 0 successo, -1 errore
int queue_sorted_insert(Queue **queue, void *data){

    //Controllo se la coda e' vuota oppure devo inserire elemento in testa
    if(*queue == NULL || fun_compare(data, (*queue)->value) <= 0){
        
        //inserisco elemento in testa
        CHECK_FUN(queue_push(queue, data), "ERRORE: impossibile eseguire la queue_push() ");
        
        return 0;
    }

    // alloco un nuovo nodo 
    Queue *newNode;

    // alloco la memoria necessaria sullo heap per il nuovo nodo
    if ((newNode = (Queue *)malloc(sizeof(Queue))) == NULL) {
        perror("ERRORE: impossibile eseguire la malloc ");
        return -1;
    }
    
    //inizializzo il nuovo nodo
    newNode->value = data;
    newNode->next = NULL;

    //uso tmp come coda temporanea per scorrere la coda passata per parametro
    Queue *tmp = *queue;

    //Itero fino a che non sono all'ultimo nodo della coda e elemento da aggiunger e' maggiore dell elemento del prossimo nodo
    while(tmp->next != NULL && fun_compare(data, tmp->next->value) > 0){
        tmp = tmp->next;
    }

    //inserisco il nuovo nodo
    newNode->next = tmp->next;
    tmp->next =newNode;

    return 0;

}

// Inserisce elemento in testa alla coda: 0 successo, -1 errore
int queue_push(Queue **queue, void *data) {
    Queue *head = NULL;

    // alloco un nuovo nodo della coda
    if ((head = (Queue *)malloc(sizeof(Queue))) == NULL) {
        perror("ERRORE: impossibile eseguire la malloc ");
        return -1;
    }
    // inizializzo newNode->data a data
    head->value = data;
    // inizializzo newNode->next alla vecchia coda
    head->next = *queue;
    // punto la coda alla testa
    *queue = head;

    return 0;
}

// Restituisce valore elemento in testa alla coda (successo), NULL se la coda e' vuota
void *queue_pop(Queue **queue) {
    //alloco e inizializzo valore elemento a NULL
    void *data = NULL;

    // controllo che la coda non sia vuota
    if (*queue != NULL) {
        // uso tmp come coda temporanea per salvare il nuovo da rimuovere
        Queue *tmp = *queue;
        // aggiorno la testa della coda
        *queue = (*queue)->next;
        // assegno a data il campo value dell elemento da rimuovere 
        data = tmp->value;
        // libero memoria allocata sull' heap per il nodo
        free(tmp);
    } 

    return data;
}

// Restituisce la lunghezza della coda
size_t len_queue(Queue *queue) {
    
    //alloco e inizializzo len a 0
    size_t len = 0;

    //Itero fino a che la coda non e' vuota
    while (queue != NULL) {
        //aumento len di 1
        len++;
        // scorro la coda
        queue = queue->next;
    }

    return len;
}


// Libera tutte le risorse allocate sull'heap presenti nella coda 
void free_queue(Queue **queue) {

    //Itero fino a che la coda non e' vuota
    while (*queue != NULL) {
        // uso tmp per fare la free del nodo
        Queue *tmp = *queue;
        // aggiorno la testa della coda
        *queue = (*queue)->next;
        // eseguo la free del campo value del nodo
        free(tmp->value);
        //eseguo la free del nodo
        free(tmp);
    }
}
