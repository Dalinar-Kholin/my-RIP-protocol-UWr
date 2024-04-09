CC=gcc

CFLAGS=-g -Wall


TARGET=program


OBJS=diagnostic.o main.o worker.o


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)


diagnostic.o: diagnostic.c prototypes.h structs.h
	$(CC) $(CFLAGS) -c diagnostic.c

main.o: main.c prototypes.h structs.h
	$(CC) $(CFLAGS) -c main.c

worker.o: worker.c prototypes.h structs.h
	$(CC) $(CFLAGS) -c worker.c


distclean:
	rm -f $(OBJS) $(TARGET)
