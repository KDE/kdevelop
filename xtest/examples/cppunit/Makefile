KDE4_INCLUDE_PATH=$(shell kde4-config -path include)

moneytest: money.h moneytest.h moneytest.cpp
	g++ -o $@ moneytest.cpp -I${KDE4_INCLUDE_PATH} -lcppunit -ldl

clean:
	@rm -v *~ moneytest
