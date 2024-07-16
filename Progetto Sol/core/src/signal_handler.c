#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#include "../include/utils.h"
#include "../include/signal_handler.h"

volatile sig_atomic_t signalFlag = 0;

static int setup_sigset(sigset_t *signalSet, int count, ...);
static int set_signal_handler(int signal, void (*handle_signal) (int));
static void handle_signal(int signal);

// Setto la nuova gestione dei segnali SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE : 0 successo, -1 errore
int setup_signal_handler() {
    
    CHECK_FUN(set_signal_handler(SIGHUP, handle_signal), "ERRORE: impossibile eseguire la set_signal_handler() di SIGHUP ");

    CHECK_FUN(set_signal_handler(SIGINT, handle_signal), "ERRORE: impossibile eseguire la set_signal_handler() di SIGINT ");

    CHECK_FUN(set_signal_handler(SIGQUIT, handle_signal), "ERRORE: impossibile eseguire la set_signal_handler() di SIGQUIT ");

    CHECK_FUN(set_signal_handler(SIGTERM, handle_signal), "ERRORE: impossibile eseguire la set_signal_handler() di SIGTERM ");

    CHECK_FUN(set_signal_handler(SIGUSR1, handle_signal), "ERRORE: impossibile eseguire la set_signal_handler() di SIGUSR1 ");

    CHECK_FUN(set_signal_handler(SIGPIPE, SIG_IGN), "ERRORE: impossibile eseguire la set_signal_handler() di SIGPIPE ");

    return 0;

}


// Blocca i segnali SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE : 0 successo, -1 errore
int block_signal() {
    //alloco una set di bit per i segnali
    sigset_t signalSet;

    //aggiungo al set SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIPIPE
    CHECK_FUN(setup_sigset(&signalSet, 6, SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE), "ERRORE: impossibile eseguire la setup_sigset() ");

    //blocco i segnali presenti nel set, controllando errore
    if(pthread_sigmask(SIG_BLOCK, &signalSet, NULL) != 0){
        perror("ERRORE: impossibile eseguire la pthread_sigmask() ");
        return -1;
    }

    return 0;
}

// Sblocca i segnali SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE : 0 successo, -1 errore
int unblock_signal() {
    //alloco una set di bit per i segnali
    sigset_t signalSet;

    //aggiungo al set SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIPIPE
    CHECK_FUN(setup_sigset(&signalSet, 6, SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE), "ERRORE: impossibile eseguire la setup_sigset() ");

    //sblocco i segnali presenti nel set, controllando errore
    if(pthread_sigmask(SIG_UNBLOCK, &signalSet, NULL) != 0){
        perror("ERRORE: impossibile eseguire la pthread_sigmask() ");
        return -1;
    }

    return 0;
}

// Blocca i segnali SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, SIGPIPE : 0 successo, -1 errore
int setup_sigset(sigset_t *signalSet, int numSignal, ...) {
    //azzero signalSet
    CHECK_FUN(sigemptyset(signalSet), "ERRORE: impossibile eseguire la sigempytset() "); 

    //alloco va_list che punta agli elmenti aggiuntivi non specificati
    va_list ap;

    //inizializzo va_list al primo elmento della lista passando per parametro ultimo valore noto (numSignal)
    va_start(ap, numSignal);

    //itero da 0 a numSignal
    for (int i = 0; i < numSignal; i++) {
        //aggiungo il segnale al set
        CHECK_FUN(sigaddset(signalSet, va_arg(ap, int)), "ERRORE: impossibile eseguire la sigaddset() ");
    }

    //libero risorse allocate da va_list
    va_end(ap);

    return 0;
}

//Setta la nuova funzione di handler di signal: 0 successo, -1 errore
int set_signal_handler(int signal, void (*handle_signal) (int)){
    
    //alloco una struct sigaction
    struct sigaction sa;
    //azzero sa
    memset(&sa, 0, sizeof(sa));
    //assegno a sa_handler la nuova funzione di handler
    sa.sa_handler = handle_signal;

    //blocco gli altri segnali che utilizzano la stessa funzione
    switch (signal) {
        case SIGHUP:
            CHECK_FUN(setup_sigset(&sa.sa_mask, 4, SIGINT, SIGQUIT, SIGTERM, SIGUSR1), "ERRORE: impossibile eseguire la setup_sigset() caso SIGHUP ");
            break;
        case SIGINT:
            CHECK_FUN(setup_sigset(&sa.sa_mask, 4, SIGHUP, SIGQUIT, SIGTERM, SIGUSR1), "ERRORE: impossibile eseguire la setup_sigset() caso SIGINT ");
            break;
        case SIGQUIT:
            CHECK_FUN(setup_sigset(&sa.sa_mask, 4, SIGINT, SIGHUP, SIGTERM, SIGUSR1), "ERRORE: impossibile eseguire la setup_sigset() caso SIGQUIT ");
            break;
        case SIGTERM:
            CHECK_FUN(setup_sigset(&sa.sa_mask, 4, SIGINT, SIGQUIT, SIGHUP, SIGUSR1), "ERRORE: impossibile eseguire la setup_sigset() caso SIGTERM ");
            break;
        case SIGUSR1:
            CHECK_FUN(setup_sigset(&sa.sa_mask, 4, SIGINT, SIGQUIT, SIGHUP, SIGTERM), "ERRORE: impossibile eseguire la setup_sigset() caso SIGUSR1 ");
            break;
    }

    //setto il nuovo signal_handler di signal
    CHECK_FUN(sigaction(signal, &sa, NULL), "ERRORE: impossibile eseguire la sigaction() ");

    //restituisco 0
    return 0;
    
}

void handle_signal(int signal){
    //setta signalFlag a signal
    signalFlag = signal;
}

