SOURCES += %{APPNAMELC}.cpp \
           main.cpp
HEADERS += %{APPNAMELC}.h
TEMPLATE = app
CONFIG += release \
          warn_on \
	  thread \
          qt
TARGET = ../bin/%{APPNAMELC}
