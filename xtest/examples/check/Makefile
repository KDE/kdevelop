KDE4_INCLUDE_PATH=$(shell kde4-config -path include)

all: check_money

money.o:money.c money.h
	gcc -c money.c

check_money.o: check_money.c money.o
	gcc -c check_money.c -I${KDE4_INCLUDE_PATH}

check_money: check_money.o money.o
	gcc -o check_money check_money.o money.o -lcheck

clean:
	rm *~ *.o check_money checklog.xml
