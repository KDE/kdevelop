SUBDIRS =

lib_LTLIBRARIES = libkdevjavapluginapi.la

INCLUDES = -I.. -I$(top_srcdir)/src -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(jni_includes) $(all_includes)

libkdevjavapluginapi_la_SOURCES = ClassStore.cpp \
			       ClassTreeNode.cpp \
			       Context.cpp \
			       DocumentationContext.cpp \
			       EditorContext.cpp \
			       KDevApi.cpp \
			       KDevAppFrontend.cpp \
			       KDevCompilerOptions.cpp \
			       Core.cpp \
			       KDevFactory.cpp \
			       KDevLanguageSupport.cpp \
			       KDevMakeFrontend.cpp \
			       KDevPart.cpp \
			       KDevProject.cpp \
			       KDevVersionControl.cpp \
			       ParsedArgument.cpp \
			       ParsedAttribute.cpp \
			       ParsedClass.cpp \
			       ParsedClassContainer.cpp \
			       ParsedContainer.cpp \
			       ParsedItem.cpp \
			       ParsedMethod.cpp \
			       ParsedParent.cpp \
			       ParsedScopeContainer.cpp \
			       ParsedStruct.cpp \
			       PersistantClassStore.cpp



EXTRA_DIST = ClassStore.h \
			       ClassTreeNode.h \
			       Context.h \
			       DocumentationContext.h \
			       EditorContext.h \
			       KDevApi.h \
			       KDevAppFrontend.h \
			       KDevCompilerOptions.h \
			       Core.h \
			       KDevFactory.h \
			       KDevLanguageSupport.h \
			       KDevMakeFrontend.h \
			       KDevPart.h \
			       KDevProject.h \
			       KDevVersionControl.h \
			       ParsedArgument.h \
			       ParsedAttribute.h \
			       ParsedClass.h \
			       ParsedClassContainer.h \
			       ParsedContainer.h \
			       ParsedItem.h \
			       ParsedMethod.h \
			       ParsedParent.h \
			       ParsedScopeContainer.h \
			       ParsedStruct.h \
			       PersistantClassStore.h


SOMAJOR = 1
SOMINOR = 0
SOSUBMINOR = 0

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
kdevjavapluginapi_HEADERS =
kdevjavapluginapidir = $(includedir)/kdevjavapluginapi

# here you should add all header files, which shouldn´t be installed
noinst_HEADERS =

libkdevjavapluginapi_la_LDFLAGS = $(all_libraries)  $(KDE_PLUGIN) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) 
libkdevjavapluginapi_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lqtjava -lkdejava 

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

KDE_OPTIONS = qtonly

METASOURCES = AUTO                                                              
