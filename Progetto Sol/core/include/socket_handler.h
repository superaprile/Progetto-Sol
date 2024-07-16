#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

int setup_socket_collector();
int setup_socket_master();
int write_socket(char *buffer, size_t bufferSize);
char *read_socket();
int close_socket_master();
int close_socket_collector();
int delete_tmp_socket();

#endif