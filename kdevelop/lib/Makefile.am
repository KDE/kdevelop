# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0 
libkdevelop_la_LIBADD = $(LIB_KPARTS)

libkdevelop_la_SOURCES = ctoolclass.cpp cproject.cpp kdevcomponent.cpp \
			 processview.cpp ctreeview.cpp ctreehandler.cpp

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS = ctoolclass.h cproject.h kdevcomponent.h \
			  processview.h ctreeview.h ctreehandler.h

METASOURCES = AUTO
