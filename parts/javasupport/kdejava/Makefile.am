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
			       PersistantClassStore.cpp \
			       CursorDocumentIface.cpp \
			       DocumentInterface.cpp \
			       EditDocumentIface.cpp \
			       Document.cpp \
			       Editor.cpp \
			       EditorInterface.cpp \
			       KDevJavaSlot.cpp



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
			       PersistantClassStore.h \
			       CursorDocumentIface.h \
			       DocumentInterface.h \
			       EditDocumentIface.h \
			       Document.h \
			       Editor.h \
			       EditorInterface.h \
			       KDevJavaSlot.h


SOMAJOR = 1
SOMINOR = 0
SOSUBMINOR = 0

libkdevjavapluginapi_la_LDFLAGS = $(all_libraries)  $(KDE_PLUGIN) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) 
libkdevjavapluginapi_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la -lqtjava -lkdejava 

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
