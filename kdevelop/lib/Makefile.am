# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0 
libkdevelop_la_LIBADD = $(LIB_KDEUI)

libkdevelop_la_SOURCES = ctoolclass.cpp cproject.cpp

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS = ctoolclass.h cproject.h 

METASOURCES = AUTO
