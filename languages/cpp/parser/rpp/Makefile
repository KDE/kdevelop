
CXXFLAGS = -DGCC_MACHINE="\"`g++ -dumpmachine`\"" -DGCC_VERSION="\"`g++ -dumpversion`\""

all:
	$(CXX) -O3 -I.. $(CXXFLAGS) -o pp main.cpp && strip pp

clean:
	rm -f pp
