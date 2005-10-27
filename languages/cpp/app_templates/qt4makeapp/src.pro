SOURCES += %{APPNAMELC}.cpp \
           main.cpp
HEADERS += %{APPNAMELC}.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = ../bin/%{APPNAMELC}
RESOURCES = application.qrc