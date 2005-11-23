
HEADERS = pp-engine-bits.h  \
	pp-engine.h  \
	pp-environment.h  \
	pp-fwd.h  \
	pp.h  \
	pp-macro-expander.h  \
	pp-macro.h  \
	pp-scanner.h  \
	pp-string.h  \
	pp-symbol.h

CXXFLAGS = -I..  -O3 -DHAVE_MMAP=1 -DGCC_MACHINE="\"`g++ -dumpmachine`\"" -DGCC_VERSION="\"`g++ -dumpversion`\""

OBJS = main.o

all: pp

pp: $(OBJS)
	$(CXX) -o$@ $(OBJS) && strip $@

clean:
	rm -f pp $(OBJS) *~


main.o: main.cpp $(HEADERS)
