CC = gcc

PROGS = myshell

all: $(PROGS)

$(PROGS): myshell.o
	$(CC) -o $(PROGS) myshell.o

myshell.o : myshell.c myshell.h
	$(CC) -c myshell.c

clean:
	rm *.o $(PROGS)
