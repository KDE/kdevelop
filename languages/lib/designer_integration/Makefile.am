INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)
METASOURCES = AUTO
libdesignerintegration_la_LDFLAGS = $(all_libraries)
lib_LTLIBRARIES = libdesignerintegration.la
libdesignerintegration_la_SOURCES = implementationwidgetbase.ui \
	implementationwidget.cpp qtdesignerintegration.cpp

langincludedirdir = $(includedir)/kdevelop/languages/designer_integration
langincludedir_HEADERS = qtdesignerintegration.h implementationwidget.h implementationwidgetbase.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevinterfaces
DOXYGEN_PROJECTNAME = KDevelop Designer Integration Support Library
include ../../../Doxyfile.am
