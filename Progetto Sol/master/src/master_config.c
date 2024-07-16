#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../../core/include/utils.h"
#include "../include/master_config.h"

//setto le variabili globali di config ai valori predefinti
size_t THREAD_WORKERS_AMOUNT = 4;
size_t CONCURRENT_QUEUE_SIZE = 8;
char *DIR_NAME = NULL;
size_t DELAY = 0;
int FIRST_FILE_INDEX = 0;

static int contains_binary(char* fileList[], int start, int end);


//legge gli argomenti opzionali passati per argomento: 0 success, -1 errore
int read_arguments(int argc, char* argv[]) {
    
    //alloco un intero per opt
    int opt;

    //itero tutti gli argomenti opzionali passati
    while ((opt = getopt(argc, argv, ": n: q: d: t:")) != -1) {

        switch (opt) {
            //caso opzione con argomento mancante
            case ':':
                fprintf(stderr, "ERRORE: opzione '-%c' richiede un argomento\n", optopt);
                return -1;
            //caso opzione non valida
            case '?':
                fprintf(stderr, "ERRORE: opzione '-%c' non valida\n", optopt);
                return -1;
            //caso -n: setto numero di thread
            case 'n':
                //salvo il numero di thread worker in THREAD_WORKERS_AMOUNT
                PARSE_VALUE(optarg, THREAD_WORKERS_AMOUNT);
                //controllo che sia un numero > 0
                if(THREAD_WORKERS_AMOUNT <= 0){
                    fprintf(stderr, "ERRORE: '%s' deve essere un numero strettamente positivo \n", optarg); 
                    return -1;                                                         
                }
                break;
            //caso -q: setto lunghezza coda concorrente
            case 'q':
                //salvo la lunghezza della coda concorrente in CONCURRENT_QUEUE_SIZE
                PARSE_VALUE(optarg, CONCURRENT_QUEUE_SIZE);
                //controllo che sia un numero > 0
                if(CONCURRENT_QUEUE_SIZE <= 0){
                    fprintf(stderr, "ERRORE: '%s' deve essere un numero  strettamente positivo \n", optarg); 
                    return -1;                                                         
                }
                break;
            //caso -d: setto nome directory
            case 'd':
                // controllo che l'argomento passato sia una directory
                if (check_file(optarg) != 2) {
                    fprintf(stderr, "ERRORE: opzione '-%c' richiede una directory come argomento\n", optopt);
                    return -1;
                }
                //parso slash al termine dell 'argomento in caso ci sia, in modo che sia "dir" che "dir/" non generino errori al momento della produzione dei task
                if(optarg[strlen(optarg)-1] == '/'){
                    optarg[strlen(optarg)-1] = '\0';
                }
                //salvo la directory in DIR_NAME
                DIR_NAME = optarg;
                break;
            //caso -t: setto delay invio richieste consecutive da parte del Master ai thread worker
            case 't':
                //salvo il numero di secondi che deve intercorrere tra produzione di task consecutivi in DELAY
                PARSE_VALUE(optarg, DELAY);
                //controllo che sia un numero >= 0
                if(DELAY < 0){
                    fprintf(stderr, "ERRORE: '%s' deve essere un numero positivo \n", optarg); 
                    return -1;                                                         
                }
                break;
        }
    }


    //controllo che venga passato almeno un file binario come argomento se non viene passata opzione -d
    if (DIR_NAME == NULL && !contains_binary(argv, optind, argc)) {
        fprintf(stderr, "ERRORE: nessun file binario passato come argomento\n");
        return -1;
    }

    //salvo indice del primo file binario in argv
    FIRST_FILE_INDEX = optind;
    
    return 0;
}


// controlla se la lista continene almeno un file binario
int contains_binary(char* fileList[], int start, int end) {
    //alloco intero per salvare tipo del file
    int typeFile;
    //itero i file binari passati per linea di comando
    for (int i = start; i < end; i++) {
        //inzializzo typeFile al valore di ritorno di check_file, controllando errore
        CHECK_FUN((typeFile = check_file(fileList[i])), "ERRORE: impossibile eseguire la check_file() ");
        //se il file e' regolare ed e' binario restituisco 1
        if (check_file(fileList[i]) == 1 && is_dat(fileList[i])) {
            return 1;
        }
    }
    //altrimenti restituisco 0
    return 0;
}
