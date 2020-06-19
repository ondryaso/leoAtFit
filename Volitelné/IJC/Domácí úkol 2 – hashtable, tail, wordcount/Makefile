# Makefile
# Řešení IJC-DU2
# oba příklady
# Datum: 12. 4. 2020
# Autor: Ondřej Ondryáš (xondry02), FIT

CC = gcc
CFLAGS = -g -std=c99 -pedantic -Wall -Wextra -O2 -fPIC
TARGETS = libhtab.a libhtab.so tail wordcount wordcount-dynamic

HTAB_MODULES = $(wildcard htab_*.c)
HTAB_OBJS = $(patsubst %.c,%.o,${HTAB_MODULES})

.PHONY:
	clean pack all run libhtab-modules

all: $(TARGETS)

tail: tail.c

wordcount: wordcount.o io.o libhtab.a
	$(CC) $(CFLAGS) -o $@ -static wordcount.o io.o -L. -lhtab

wordcount-dynamic: wordcount.o io.o libhtab.so
	$(CC) $(CFLAGS) -o $@ wordcount.o io.o -L. -lhtab

wordcount.o: wordcount.c io.h htab.h
io.o: io.c

libhtab.a: ${HTAB_OBJS}
	ar crs $@ ${HTAB_OBJS}

libhtab.so: ${HTAB_OBJS}
	$(CC) $(CFLAGS) -shared -o $@ ${HTAB_OBJS}

${HTAB_OBJS}: ${HTAB_MODULES} htab.h htab_priv.h
	$(CC) $(CFLAGS) -c ${HTAB_MODULES}

clean:
	$(RM) *.o $(TARGETS) xondry02.zip

pack:
	zip xondry02.zip *.h *.c Makefile
