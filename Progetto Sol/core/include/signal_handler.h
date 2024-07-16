#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

extern volatile sig_atomic_t signalFlag;

int setup_signal_handler();
int block_signal();
int unblock_signal();

#endif