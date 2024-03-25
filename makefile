CC=gcc
CFLAGS=-I. -Wall
DEPS = moteur.h
OBJ_SERVEUR = serveur.o moteur.o
OBJ_CLIENT = client.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

serveur: $(OBJ_SERVEUR)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o serveur client
