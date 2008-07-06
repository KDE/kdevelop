SOURCES += main.cpp
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = %{APPNAMELC}
DESTDIR = ../bin
QT -= gui
