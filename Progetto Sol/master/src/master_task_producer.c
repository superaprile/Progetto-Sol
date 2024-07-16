#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "../../core/include/utils.h"
#include "../../core/include/socket_handler.h"
#include "../../core/include/signal_handler.h"
#include "../include/master_config.h"
#include "../include/master_task_handler.h"
#include "../include/master_task_producer.h"

// Se fun ha generato un errore, stampo messaggio di errore e restituisco -1
#define CHECK_FUN_R(fun, message)                                                       \
    {                                                                                   \
        if (fun == -1) {                                                                \
            perror(message);                                                            \
            CHECK_FUN(closedir(directory), "ERRORE: impossibile eseguire closedir() "); \
            return -1;                                                                  \
        }                                                                               \
    }

// static void process_dir();
static int process_dir(char *dirName);


// Esegue la push dei path relativi dei file binari sulla coda concorrente. Restituisce: 0 successo, -1 errore/segnale di terminazione
int produce_tasks(char *fileList[], int end) {

    // controllo se e' stata passata la directory come argomento opzionale
    if (DIR_NAME != NULL) {
        
        //navigo la directory ricorsivamente eseguendo la push dei path relativi dei file binari presenti sulla coda concorrente
        if (process_dir(DIR_NAME) == -1){

            //controllo se ho ricevuto un segnale di terminazione
            if(signalFlag != 0 && signalFlag != SIGUSR1){
                //segnalo ai thread worker di terminare chiudendo la coda concorrente
                close_conccurrent_queue();
                //restituisco 0
                return 0;
            } else {
                //se process_dir ha generato un errore restituisco -1
                return -1;
            }
            
        }
    }   


    //alloco intero per salvare valore di ritorno di check_file()
    int typeFile;

    //setto sleep rimanente a DELAY
    unsigned int remainingSleep = DELAY;

    //itero tutti i file binari passati per argomento
    for (size_t i = FIRST_FILE_INDEX; i < end; i++) {
        
        //salvo il tipo del file in typeFile e gestisco il caso di errore
        CHECK_FUN((typeFile = check_file(fileList[i])), "ERRORE: impossibile eseguire la check_file() ");
        
        //controllo che siano effetivamente file binari
        if (typeFile == 1 && is_dat(fileList[i])) {
            
            //controllo se ho ricevuto dei segnali
            switch (signalFlag){
            
            //caso 0: nessun segnale ricevuto
            case 0:
                
                //eseguo la push del path relativo del file sulla coda concorrente
                CHECK_FUN(push_task(fileList[i]), "ERRORE: impossibile eseguire la push_task() ");
                break;
            
            //caso SIGUSR1: ricevuto segnale SIGUSR1
            case SIGUSR1:
                
                //eseguo una scrittura sulla socket per segnalare al processore Collector di stampare i task fino ad ora processati
                CHECK_FUN(write_socket("STAMPA", strlen("STAMPA") + 1), "ERRORE: impossibile eseguire la write_socket() ");

                //resetto il flag del segnale
                signalFlag = 0;
                
                //eseguo la push del path relativo del file sulla coda concorrente
                CHECK_FUN(push_task(fileList[i]), "ERRORE: impossibile eseguire la push_task() ");
                break;
            
            //caso default: ricevuto un segnale tra: SIGHUP, SIGINT, SIGTERM, SIGQUIT, SIGPIPE
            default:
                //segnalo ai thread worker di terminare chiudendo la coda concorrente
                close_conccurrent_queue();
                //restituisco 0
                return 0;
            }

            //itero fino a che non finisco la sleep specificata dall'opzione -t passata da linea di comando (default = 0)
            while ((remainingSleep = sleep(remainingSleep)) != 0) {
                //ricevuto il segnale SIGUSR1
                if (signalFlag == SIGUSR1) {
                    // eseguo una scrittura sulla socket per segnalare al processore Collector di stampare i task fino ad ora processati
                    CHECK_FUN(write_socket("STAMPA", strlen("STAMPA") + 1), "ERRORE: impossibile eseguire la write_socket() ");
                    //resetto signalFlag
                    signalFlag = 0;
                }
                //ricevuto un segnale di terminazione
                if (signalFlag != 0 && signalFlag != SIGUSR1) {
                    //segnalo ai thread worker di terminare chiudendo la coda concorrente
                    close_conccurrent_queue();
                    //restituisco 0
                    return 0;
                }
            }

            //resetto reminingSleep a DELAY per la prossima push
            remainingSleep = DELAY;
        }

        // Ignorato: caso file non esistente/non binario
        // } else {
            
        //     // se il file non e' binario segnalo ai thread worker di terminare chiudendo la coda concorrente
        //     fprintf(stderr, "ERRORE: '%s' non e' un file binario\n", fileList[i]);
        //     return -1;
        // }
            
        
    }

    //una volta di iterare i file passati da linea di comando segnalo ai thread worker di terminare chiudendo la coda 
    close_conccurrent_queue();

    return 0;
}


// Itera la directory ricorsivamente ed esegue la push di tutti i path relativi dei file binari presenti sulla coda concorrente: 0 successo, -1 errore/segnale terminazione
int process_dir(char *dirName) {

    // alloco puntatore dove salvare stream directory
    DIR *directory;

    // alloco struct in cui conservo il valore di ritorno di readdir
    struct dirent *file;

    // eseguo la open della dir corrente salvando lo stream DIR in directory
    if ((directory = opendir(dirName)) == NULL) {
        perror("ERRORE: impossibile eseguire opendir() ");
        return -1;
    }

    //resetto errno per gesite errore readdir
    errno = 0;

    //setto remainingSleep a DELAY
    unsigned int remainingSleep = DELAY;
    
    //itero tutti i file presenti in directory
    while ((file = readdir(directory)) != NULL) {

        // controllo che il file non sia ne la dir precendete (..) ne quella corrente (..)
        if (strncmp(file->d_name, ".", strlen(".") + 1) != 0 && strncmp(file->d_name, "..", strlen("..") + 1) != 0) {
           
            //alloco un array di char per salvare il path relativo del file, inizializzandolo a "\0" per la strncat
            char fileName[PATH_SIZE] = "\0";
            //concateno il nome della dir corrente al path relativo del file
            strncat(fileName, dirName, PATH_SIZE);
            //concateno lo '/' al path relativo del file
            strncat(fileName, "/", PATH_SIZE - strlen(fileName));
            //concateno il nome del file al path relativo del file 
            strncat(fileName, file->d_name, PATH_SIZE - strlen(fileName) - 1);

            //alloco un intero per salvare il tipo del file
            int typeFile; 

            //salvo il tipo di file in typeFile controllando il caso di errore
            CHECK_FUN_R((typeFile = check_file(fileName)), "ERRORE: impossibile eseguire la check_file() ");

            //controllo se il file si tratta di un directory
            if (typeFile == 2) {
                //caso dir: Itero tutti i file presenti in filename, controllando error
                CHECK_FUN_R(process_dir(fileName), "ERRORE: impossibile eseguire process_dir() ");

            } else {
                // controllo se il file si tratta di un file binario
                if (typeFile == 1 && is_dat(fileName)) {

                    //controllo se ho ricevuto dei segnali
                    switch (signalFlag) {
                        //caso 0: nessun segnale ricevuto
                        case 0:
                            //eseguo la push del path relativo del file sulla coda concorrente
                            CHECK_FUN_R(push_task(fileName), "ERRORE: impossibile eseguire la push_task() ");
                            break;

                        //caso SIGUSR1: ricevuto segnale SIGUSR1
                        case SIGUSR1:
                            
                            //eseguo una scrittura sulla socket per segnalare al processore Collector di stampare i task fino ad ora processati
                            CHECK_FUN_R(write_socket("STAMPA", strlen("STAMPA") + 1), "ERRORE: impossibile eseguire la write_socket() ");

                            //resetto signalFlag
                            signalFlag = 0;

                            //eseguo la push del path relativo del file sulla coda concorrente
                            CHECK_FUN_R(push_task(fileName), "ERRORE: impossibile eseguire la push_task() ");

                            break;

                        //caso segnale di terminazione: ricevuto un segnale tra: SIGHUP, SIGINT, SIGTERM, SIGQUIT, SIGPIPE
                        default:
                            // eseguo la close della dir corrente
                            CHECK_FUN(closedir(directory), "ERRORE: impossibile eseguire closedir() ");
                            //restituisco -1
                            return -1;
                    }
                    //Itero fino a che non finisco la sleep specificata dall'opzione -t passata da linea di comando (default = 0)
                    while ((remainingSleep = sleep(remainingSleep)) != 0) {
                        //controllo se ho ricevuto il segnale SIGUSR1
                        if (signalFlag == SIGUSR1) {
                            //eseguo una scrittura sulla socket per segnalare al processore Collector di stampare i task fino ad ora processati
                            CHECK_FUN_R(write_socket("STAMPA", strlen("STAMPA") + 1), "ERRORE: impossibile eseguire la write_socket() ");
                            //resetto signalFlag
                            signalFlag = 0;
                        }
                        // controllo se ho ricevuto un segnale di terminazione
                        if (signalFlag != 0 && signalFlag != SIGUSR1) {
                            // eseguo la close della dir corrente
                            CHECK_FUN(closedir(directory), "ERRORE: impossibile eseguire closedir() ");
                            //restituisco -1
                            return -1;
                        }
                    }

                    //resetto remainingSleep a Delay per la prossima push
                    remainingSleep = DELAY;
                }
                // Ignorato: caso file non esistente/non binario
                // } else {
                //     fprintf(stderr, "ERRORE: '%s' non e' un file binario\n", fileName);
                //     return -1;
                // }
            }
        }

        // resetto errno
        errno = 0;
    }

    // controllo se la readdir ha generato un errore
    if (errno != 0) {
        perror("ERRORE: impossibile eseguire readdir() ");
        CHECK_FUN(closedir(directory), "ERRORE: impossibile eseguire closedir() ");
        return -1;
    }

    // eseguo la close della dir corrente
    CHECK_FUN(closedir(directory), "ERRORE: impossibile eseguire closedir() ");

    return 0;
}


