# Makefile
# Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

CXX = g++
CPPFLAGS = -g -std=gnu++17 -Wall

MODULES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,${MODULES})

.PHONY:
	clean all pack

all: secret

secret: ${OBJS}
	$(CXX) $(CPPFLAGS) -o $@ ${OBJS} -lcrypto

${OBJS}: ${MODULES}
	$(CXX) $(CPPFLAGS) -c ${MODULES}

clean:
	$(RM) *.o secret xondry02.tar

pack:
	tar -cf xondry02.tar *.h *.cpp secret.1 manual.pdf Makefile
