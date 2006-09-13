SOURCES += main.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = ../bin/%{APPNAMELC}

QT -= gui
