####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkwrite.la


INCLUDES = $(all_includes)


LDFLAGS = 

libkwrite_la_METASOURCES = AUTO

libkwrite_la_SOURCES = kwview.cpp kwdoc.cpp kwdialog.cpp highlight.cpp 

SUBDIRS = pix 

EXTRA_DIST = highlight.cpp highlight.h kwdialog.cpp kwdialog.h kwdoc.cpp kwdoc.h kwview.cpp kwview.h 

####### kdevelop will overwrite this part!!! (end)############
SOMAJOR = 1
SOMINOR = 0
SOSUBMINOR = 0

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn´t be installed
noinst_HEADERS =

libkwrite_la_LDFLAGS = $(all_libraries) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) $(LDFLAGS)

# add here the libraries which have to be include
# for building the shared lib
#
#  please consider also the usage of the make environment variables
#  like
#  $(LIB_KDEUI), $(LIB_QT), $(LIB_HTML), etc.
#  instead of the hardcoded entries, which have placed
#  in here.
#  The following line is only a suggestion!
#
libkwrite_la_LIBADD = -lkfile -lkhtml -lkdeui -lkdecore $(LIB_QT)

