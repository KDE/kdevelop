INCLUDES = \
    -I$(top_srcdir)/lib/interfaces \
    -I$(top_srcdir)/lib/project \
    -I$(top_srcdir)/lib/util \
    $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = \
    libprojectmanager_interfaces.la

libprojectmanager_interfaces_la_SOURCES = kdevprojectimporter.cpp \
	kdevprojectmodel.cpp kdevprojectbuilder.cpp kdevprojecteditor.cpp

libprojectmanager_interfaces_la_LDFLAGS = \
    -no-undefined $(all_libraries) -version-info 1:0

libprojectmanager_interfaces_la_LIBADD = \
    $(top_builddir)/lib/libkdevelop.la

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = \
    kdevelopprojectimporter.desktop \
    kdevelopprojectbuilder.desktop
