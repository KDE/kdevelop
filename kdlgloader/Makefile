#QTDIR	=	/usr/lib/qt
#KDEDIR	=	/opt/kde

INC	=	-I$(QTDIR)/include -I$(KDEDIR)/include
CXXFLAGS	= -shared -Wall

LIBINSTALLDIR=	$(KDEDIR)/lib
INCINSTALLDIR=	$(KDEDIR)/include

#.PHONY:	all install

all:
	g++ $(CXXFLAGS) $(INC) kdlgldr.cpp kdlgldr_itemstuff.cpp kdlgptrdb.cpp -o libkdlgloader.so

install:
	install -m 555 libkdlgloader.so $(LIBINSTALLDIR)
	install -m 555 kdlgloader.h $(INCINSTALLDIR)

