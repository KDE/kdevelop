
INCLUDES = $(all_includes)
#LDFLAGS	= $(all_libraries) -L/usr/lib
LDADD = -lkparts -lkdecore -lkdeui -lktexteditor

check_PROGRAMS = manager

manager_SOURCES = app.cpp docmanager.cpp viewmanager.cpp
manager_LDADD = -lkparts $(LIB_KFILE)

managerdir = $(kde_datadir)/manager
manager_DATA = manager.rc

METASOURCES = AUTO



