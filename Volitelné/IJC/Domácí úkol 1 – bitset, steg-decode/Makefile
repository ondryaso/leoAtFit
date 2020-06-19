# Makefile
# Řešení IJC-DU1
# oba příklady
# Datum: 13. 3. 2020
# Autor: Ondřej Ondryáš (xondry02), FIT
# Poznámka: cíl run kompiluje a spouští pouze program z příkladu A,
#           jak je určeno v zadání

CC = gcc
CFLAGS = -g -std=c11 -pedantic -Wall -Wextra -O2
TARGETS = primes primes-i steg-decode

.PHONY:
	clean pack all run

all: $(TARGETS)

run: primes primes-i
	ulimit -s 65535; ./primes
	ulimit -s 65535; ./primes-i

primes: primes.o eratosthenes.o error.o
	$(CC) $(CFLAGS) -o $@ -lm $^
primes.o: primes.c bitset.h eratosthenes.h error.h

primes-i: primes-i.o eratosthenes-i.o error.o
	$(CC) $(CFLAGS) -o $@ -lm $^
primes-i.o: primes.c bitset.h eratosthenes.h error.h
	$(CC) $(CFLAGS) -DUSE_INLINE -o $@ -c primes.c

steg-decode: steg-decode.o error.o ppm.o eratosthenes.o
	$(CC) $(CFLAGS) -o $@ -lm $^
steg-decode.o: steg-decode.c error.h bitset.h ppm.h eratosthenes.h

eratosthenes.o: eratosthenes.c bitset.h eratosthenes.h error.h
eratosthenes-i.o: eratosthenes.c bitset.h eratosthenes.h error.h
	$(CC) $(CFLAGS) -DUSE_INLINE -o $@ -c eratosthenes.c

ppm.o: ppm.c ppm.h error.h

error.o: error.c error.h
	$(CC) $(CFLAGS) -c error.c -o $@

clean:
	$(RM) *.o $(TARGETS) xondry02.zip

pack:
	zip xondry02.zip *.h *.c Makefile