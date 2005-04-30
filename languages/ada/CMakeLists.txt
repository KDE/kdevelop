SUBDIRS( app_templates file_templates doc )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/antlr ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR})

ADD_DEFINITIONS(-fexceptions)

SET(kdevadasupport_PART_SRCS
adasupportpart.cpp
problemreporter.cpp
backgroundparser.cpp
addclass.cpp
ada_utils.cpp
adasupport.cpp
AdaLexer.cpp
AdaParser.cpp
AdaTreeParserSuper.cpp
AdaStoreWalker.cpp
)

SET( kdevadasupport_UI
addclassdlg.ui
configproblemreporter.ui
)

KDE_AUTOMOC(${kdevadasupport_PART_SRCS})

KDE_ADD_UI_FILES(kdevadasupport_PART_SRCS ${kdevadasupport_UI} )

ADD_LIBRARY(kdevadasupport SHARED ${kdevadasupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevadasupport ${KDEV_LANG_LIBS})

KDE_TARGET_LINK_CONV_LIBRARIES(kdevadasupport antlr)

#original Makefile.am contents follow:

## Here resides the Ada support part.
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#INCLUDES = -I$(top_srcdir)/lib/antlr -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	$(all_includes)
#SUBDIRS = app_templates file_templates doc
#
#
#kde_module_LTLIBRARIES = libkdevadasupport.la
#libkdevadasupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
#libkdevadasupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la 				$(top_builddir)/lib/antlr/src/libantlr.la
#
#libkdevadasupport_la_SOURCES = adasupportpart.cpp problemreporter.cpp backgroundparser.cpp addclassdlg.ui addclass.cpp configproblemreporter.ui ada_utils.cpp adasupport.cpp AdaLexer.cpp AdaParser.cpp AdaTreeParserSuper.cpp AdaStoreWalker.cpp
#
#METASOURCES = AUTO
#
#EXTRA_DIST = ada.g ada.tree.g ada.store.g
#
### The following three rules assume that you have Java installed,
### ANTLR installed, and you have the antlr jar in your CLASSPATH.
#
##AdaLexer.hpp AdaLexer.cpp AdaParser.hpp AdaParser.cpp: ada.g
##	antlr   ada.g
#
##AdaTreeParserSuper.hpp AdaTreeParserSuper.cpp: ada.tree.g
##	antlr   ada.tree.g
#
##AdaStoreWalker.hpp AdaStoreWalker.cpp: ada.store.g
##	antlr -glib ada.tree.g  ada.store.g
#
#genparser:
#	antlr   ada.g && antlr   ada.tree.g && antlr -glib ada.tree.g  ada.store.g
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevadasupport.desktop
#
#rcdir = $(kde_datadir)/kdevadasupport
#rc_DATA = kdevadasupport.rc
#
#KDE_OPTIONS = nofinal
