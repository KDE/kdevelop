####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkdevelopvc.la


INCLUDES = $(all_includes)


LDFLAGS = 

libkdevelopvc_la_LDFLAGS = -version-info 2:0

libkdevelopvc_la_METASOURCES = AUTO

libkdevelopvc_la_SOURCES = versioncontrol.cpp cvsdialog.cpp cvs.cpp commitdialog.cpp


EXTRA_DIST = commitdialog.cpp commitdialog.h cvs.cpp cvs.h cvsdialog.cpp cvsdialog.h versioncontrol.cpp versioncontrol.h 

####### kdevelop will overwrite this part!!! (end)############
SOMAJOR = 1
SOMINOR = 0
SOSUBMINOR = 0

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn´t be installed
noinst_HEADERS =

libkdevelopvc_la_LDFLAGS = $(all_libraries) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) $(LDFLAGS)

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
libkdevelopvc_la_LIBADD = -lkfile -lkhtml -lkdeui -lkdecore $(LIB_QT)

