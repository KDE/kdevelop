SUBDIRS( file_templates app_templates compiler doc )

SET(kdevpascalsupport_PART_SRCS
pascalsupport_part.cpp
PascalLexer.cpp
PascalParser.cpp
PascalStoreWalker.cpp
backgroundparser.cpp
problemreporter.cpp
)

KDE_AUTOMOC(${kdevpascalsupport_PART_SRCS})

SET(
configproblemreporter.ui
)

KDE_ADD_UI_FILES(kdevpascalsupport_PART_SRCS ${kdevpascalsupport_UI}

ADD_LIBRARY(kdevpascalsupport SHARED ${kdevpascalsupport_PART_SRCS})



#original Makefile.am contents follow:

#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#INCLUDES = -I$(top_srcdir)/lib/antlr -I$(top_srcdir)/lib/catalog \
#	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	-I$(top_builddir)/bdb/build_unix -I$(top_srcdir)/bdb/include $(all_includes)
#SUBDIRS = file_templates app_templates compiler doc
#
#kde_module_LTLIBRARIES = libkdevpascalsupport.la
#libkdevpascalsupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
#libkdevpascalsupport_la_LIBADD = $(top_builddir)/bdb/build_unix/libdb.a $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/antlr/src/libantlr.la $(top_builddir)/lib/catalog/libkdevcatalog.la
#
#libkdevpascalsupport_la_SOURCES = pascalsupport_part.cpp PascalLexer.cpp PascalParser.cpp PascalStoreWalker.cpp backgroundparser.cpp configproblemreporter.ui problemreporter.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevpascalsupport.desktop
#
#rcdir = $(kde_datadir)/kdevpascalsupport
#rc_DATA = kdevpascalsupport.rc
#
#genparser:
#	antlr   pascal.g && antlr pascal.tree.g
#
### The following rules assume that you have Java and ANTLR installed,
##PascalLexer.hpp PascalLexer.cpp PascalParser.hpp PascalParser.cpp: pascal.g
##	antlr pascal.g
#
##PascalStoreWalker.hpp PascalStoreWalker.cpp: pascal.tree.g
##	antlr pascal.tree.g
#
#templatedir = ${kde_datadir}/kdevabbrev/templates
#template_DATA = pascaltemplates

#######################
#previous CMakeLists.txt contents follow:

