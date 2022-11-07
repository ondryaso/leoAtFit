# ICP project
# Authors: František Nečas (xnecas27)
#          Ondřej Ondryáš (xondry02)

.PHONY: all run doxygen pack clean

BUILDDIR = cmake-build

all: ICP

run: ICP
	./ICP

doxygen:
	mkdir -p doc
	doxygen Doxyfile

ICP:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake .. && make
	cp $(BUILDDIR)/ICP .

clean:
	rm -rf $(BUILDDIR)
	rm -f ICP

pack:
	rm -rf doc/html
	zip -r 1-xnecas27-xondry02.zip src examples README.txt Makefile Doxyfile CMakeLists.txt assets doc
