# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files

SUBDIRS = projectmanagement

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0 
libkdevelop_la_LIBADD = $(top_srcdir)/kdevelop/classparser/libkdevclassstore.la \
			$(top_srcdir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
			$(LIB_KPARTS)

libkdevelop_la_SOURCES = kdevcomponent.cpp kdevversioncontrol.cpp \
			 kdevlanguagesupport.cpp kdevcompileroptions.cpp \
			 ctoolclass.cpp cproject.cpp processview.cpp

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS = kdevcomponent.h kdevversioncontrol.h \
			  kdevlanguagesupport.h kdevcompileroptions.h \
			  ctoolclass.h cproject.h processview.h ctreehandler.h

METASOURCES = AUTO
