# makefile for the systwit project
#
# optional with plugins building

CC=gcc
CFLAGS=-g
PCFLAGS=-g -c -Wall -Werror -fpic

all: build
	@make -s clean

build: systwit.o plugin_manager.o
	$(CC) $(CFLAGS) -o bin/systwit systwit.o plugin_manager.o -lrt -ldl

plugin_manager.o: lib/plugin_manager.c lib/plugin_manager.h
	$(CC) $(CFLAGS) -c lib/plugin_manager.c -lrt -ldl

systwit.o: systwit.c lib/plugin_manager.h
	$(CC) $(CFLAGS) -c systwit.c -lrt -ldl

syslogger: plugins/syslogger.c
	$(CC) $(PCFLAGS) plugins/syslogger.c -lrt -ldl
	$(CC) -shared -o bin/plugins/libsyslogger.so syslogger.o

clean:
	rm -f *.o
	rm -f lib/*.o

clean_plugins:
	rm -f plugins/*.o
