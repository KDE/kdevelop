####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libgfxview.la


INCLUDES = $(all_includes)


LDFLAGS = 

libgfxview_la_METASOURCES = AUTO

libgfxview_la_SOURCES = GfxClassTreeWindow.cpp GfxClassTreeScrollView.cpp GfxClassTree.cpp GfxClassBox.cpp 


EXTRA_DIST = GfxClassBox.cpp GfxClassBox.h GfxClassTree.cpp GfxClassTree.h GfxClassTreeScrollView.cpp GfxClassTreeScrollView.h GfxClassTreeWindow.cpp GfxClassTreeWindow.h 

####### kdevelop will overwrite this part!!! (end)############
SOMAJOR = 1
SOMINOR = 0
SOSUBMINOR = 0

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn´t be installed
noinst_HEADERS =

libgfxview_la_LDFLAGS = $(all_libraries) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) $(LDFLAGS)

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
libgfxview_la_LIBADD = -lkfile -lkhtml -lkdeui -lkdecore $(LIB_QT)

