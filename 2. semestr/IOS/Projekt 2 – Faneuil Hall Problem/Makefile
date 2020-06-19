# Makefile
# IOS, projekt 2
# Autor: Ondřej Ondryáš
# Datum: 19. 4. 2020

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LDFLAGS = -lpthread

.PHONY: clean

proj2: main.o judge.o immigrant.o immigrant_generator.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) -f proj2.zip *.o proj2

pack:
	zip proj2.zip *.c *.h Makefile