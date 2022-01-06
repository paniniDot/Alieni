CFLAGS=-ansi -Wpedantic -Wall -Werror -D_THREAD_SAFE -D_REENTRANT -D_POSIX_C_SOURCE=200112L
LIBRARIES=-lpthread
LFLAGS=

all: alieni.exe

alieni.exe: alieni.o DBGpthread.o
	gcc ${LFLAGS} -o alieni.exe alieni.o DBGpthread.o ${LIBRARIES}

alieni.o: alieni.c DBGpthread.h
	gcc -c ${CFLAGS} alieni.c

DBGpthread.o: DBGpthread.c printerror.h
	gcc -c ${CFLAGS} DBGpthread.c

.PHONY: clean run

clean:
	rm -f *.exe *.o *~ core

run: alieni.exe
	./alieni.exe


