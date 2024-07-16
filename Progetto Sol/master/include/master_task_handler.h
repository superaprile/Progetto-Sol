#ifndef MASTER_TASK_HANDLER_H
#define MASTER_TASK_HANDLER_H

void create_conccurent_queue();
int push_task(char *fileName);
char *pop_task();
void close_conccurrent_queue();

#endif