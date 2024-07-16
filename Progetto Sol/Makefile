CC = gcc

CFLAGS = -Wall -pedantic -std=c99 -g 

EXE = farm

LIBS = -pthread

OBJS = build/objs/master.o build/objs/collector.o build/objs/master_config.o build/objs/master_farm.o build/objs/master_task_producer.o build/objs/master_task_handler.o build/objs/master_task_worker.o build/objs/queue_utils.o build/objs/signal_handler.o build/objs/socket_handler.o build/objs/utils.o

.PHONY : test testSignal cleanProject cleanFile cleanAll

$(EXE) : $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $(EXE)

build/objs/master.o : master/src/master.c build/objs/master_farm.o build/objs/collector.o build/objs/signal_handler.o build/objs/socket_handler.o build/objs/master_config.o
	$(CC) $(CFLAGS) -c $< -o $@

build/objs/collector.o : collector/src/collector.c collector/include/collector.h build/objs/utils.o build/objs/queue_utils.o build/objs/socket_handler.o
	$(CC) $(CFLAGS) -c $< -o $@

build/objs/master_config.o : master/src/master_config.c master/include/master_config.h build/objs/utils.o
	$(CC) $(CFLAGS) -c $< -o $@

build/objs/master_farm.o : master/src/master_farm.c master/include/master_farm.h core/include/pthread_utils.h build/objs/master_config.o build/objs/master_task_worker.o build/objs/master_task_handler.o build/objs/master_task_producer.o build/objs/signal_handler.o
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

build/objs/master_task_producer.o : master/src/master_task_producer.c master/include/master_task_producer.h build/objs/master_config.o build/objs/signal_handler.o build/objs/socket_handler.o build/objs/master_task_handler.o build/objs/utils.o
	$(CC) $(CFLAGS) -c $< -o $@

build/objs/master_task_handler.o : master/src/master_task_handler.c master/include/master_task_handler.h core/include/pthread_utils.h build/objs/queue_utils.o build/objs/master_config.o
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

build/objs/master_task_worker.o : master/src/master_task_worker.c master/include/master_task_worker.h build/objs/master_task_handler.o build/objs/socket_handler.o
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

build/objs/queue_utils.o : core/src/queue_utils.c core/include/queue_utils.h build/objs/utils.o
	$(CC) $(CFLAGS) -c $< -o $@

build/objs/signal_handler.o : core/src/signal_handler.c core/include/signal_handler.h build/objs/utils.o
	$(CC) $(CFLAGS) -c $< -o $@

build/objs/socket_handler.o : core/src/socket_handler.c core/include/socket_handler.h core/include/pthread_utils.h build/objs/utils.o
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

build/objs/utils.o : core/src/utils.c core/include/utils.h 
	$(CC) $(CFLAGS) -c $< -o $@

generafile : build/generafile.c
	$(CC) $(CFLAGS) $< -o $@

datFiles: generafile
	@chmod +x build/datFiles.sh
	@build/datFiles.sh

test: farm generafile
	@chmod +x build/test.sh
	@build/test.sh
	@-rm *.dat expected.txt generafile
	@-rm -r testdir/

testSignal: farm generafile
	@chmod +x build/testSignal.sh
	@build/testSignal.sh
	@-rm *.dat expected.txt generafile
	@-rm -r testdir/

cleanProject:
	@-rm $(OBJS) $(EXE) build/objs/generafile.o
	@echo "Clean progetto eseguita con successo!"

cleanFile:
	@-rm *.dat expected.txt generafile
	@-rm -r testdir/
	@echo "Clean file binari eseguita con successo!"

cleanAll: cleanProject cleanFile 
