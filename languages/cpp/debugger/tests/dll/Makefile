
all: main libhelper.so

main: main.cpp
	g++ -g -o main main.cpp -ldl
	
libhelper.so: helper.cpp
	g++ -g -o libhelper.so -fPIC -shared helper.cpp