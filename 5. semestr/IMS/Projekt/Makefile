# Makefile
# Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)

CXX = g++
CPPFLAGS = -g -std=gnu++17 -Wall -Wno-unknown-pragmas -O2 -I/usr/local/include

MODULES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,${MODULES})

TARGET = 05_xondry02_xnecas27.zip

.PHONY:
	clean all pack run

all: sim

sim: ${OBJS}
	$(CXX) $(CPPFLAGS) -o $@ ${OBJS} -L/usr/local/lib64 -lsimlib -lm

battery_improvement_provider.o: battery_improvement_provider.cpp \
 battery_improvement_provider.h util.h sim_config.h
battery_scenario.o: battery_scenario.cpp battery_scenario.h \
 load_data_provider.h energy_cost_provider.h inflation_provider.h util.h \
 battery_improvement_provider.h energy_anomaly.h sim_config.h
energy_anomaly.o: energy_anomaly.cpp energy_anomaly.h util.h sim_config.h
energy_cost_provider.o: energy_cost_provider.cpp energy_cost_provider.h
inflation_provider.o: inflation_provider.cpp inflation_provider.h util.h \
 sim_config.h
load_data_provider.o: load_data_provider.cpp load_data_provider.h
main.o: main.cpp sim_config.h base_scenario.h load_data_provider.h \
 energy_cost_provider.h inflation_provider.h util.h energy_anomaly.h \
 battery_scenario.h battery_improvement_provider.h

clean:
	$(RM) *.o sim $(TARGET)

run: sim
	./sim

pack:
	zip $(TARGET) *.cpp *.h *.tsv Makefile dokumentace.pdf
