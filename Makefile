CC=gcc
CFLAGS=-I. -Ijansson/src
DEPS=
OBJ=src/gistit.o
LIBS=-lcurl

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gistit: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) jansson/src/.libs/libjansson.a 

.PHONY: clean

clean:
	rm -f src/*.o gistit
