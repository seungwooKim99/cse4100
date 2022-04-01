CC = gcc

PROGS = 20181610

all: $(PROGS)

$(PROGS): myshell.o
	$(CC) -o $(PROGS) myshell.o

myshell.o :
	$(CC) -c myshell.c

clean:
	rm *.o $(PROGS)