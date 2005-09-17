TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= %{APPNAMELC}.h
SOURCES		= main.cpp %{APPNAMELC}.cpp
INTERFACES	= %{APPNAMELC}base.ui
TARGET		= %{APPNAMELC}
INCLUDEPATH     += $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe

