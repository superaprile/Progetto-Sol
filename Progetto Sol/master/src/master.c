#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../collector/include/collector.h"
#include "../../core/include/signal_handler.h"
#include "../../core/include/socket_handler.h"
#include "../include/master_config.h"
#include "../include/master_farm.h"

int main(int argc, char *argv[]) {

    //alloco intero salvare pid di ritorno dalla fork()
    int pid;

    //blocco i segnali SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE
    if (block_signal() == -1) {
        perror("ERRORE: impossibile eseguire la block_signal() ");
        exit(EXIT_FAILURE);
    }

    //eseguo la fork per creare il processo collector
    if ((pid = fork()) == -1) {
        perror("ERRORE: impossibile eseguire la fork() ");
        exit(EXIT_FAILURE);
    }

    //controllo il pid per distinguere tra Master (pid != 0) e Collector (pid == 0)
    if (pid != 0) {
        //mi connetto alla socket per comunicare con il processo Collector
        if(setup_socket_master() == -1){
            perror("ERRORE: impossibile eseguire setup_socket_master() ");
            exit(EXIT_FAILURE);
        }

        //leggo gli argomenti passati da linea di comando
        if(read_arguments(argc, argv) == -1){
            perror("ERRORE: impossibile eseguire la read_arguments() ");
            exit(EXIT_FAILURE);
        }

        //avvio la farm 
        if(create_farm(argv, argc) == -1){
            perror("ERRORE: impossibile eseguire la create_farm() ");
            exit(EXIT_FAILURE);
        }

        //chiudo la socket
        if(close_socket_master() == -1){
            perror("ERRORE: impossibile eseguire la close_socket_master() ");
            exit(EXIT_FAILURE);
        }

        //aspetto la terminazione del Colletor
        if (waitpid(pid, NULL, 0) == -1) {
            perror("ERRORE: impossibile eseguire la waitpid() ");
            exit(EXIT_FAILURE);
        }

    } else {

        //avvio il Collector
        start_collector();
    }

    return 0;
}
