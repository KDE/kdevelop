
CXXFLAGS = -I..  -g -DGCC_MACHINE="\"`g++ -dumpmachine`\"" -DGCC_VERSION="\"`g++ -dumpversion`\""

OBJS = main.o

all: pp

pp: $(OBJS)
	$(CXX) -o$@ $(OBJS) # && strip $@

clean:
	rm -f pp $(OBJS) *~

main.cpp: pp.h pp-environment.h  pp-macro.h    pp-string.h  preprocessor.h \
	pp-fwd.h          pp-macro-expander.h  pp-scanner.h  pp-symbol.h
