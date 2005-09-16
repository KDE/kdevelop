INCLUDES = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/util \
	$(all_includes)

kde_module_LTLIBRARIES = libkdevprojectmanager.la
libkdevprojectmanager_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevprojectmanager_la_LIBADD = \
	$(top_builddir)/lib/libkdevelop.la

libkdevprojectmanager_la_SOURCES = \
	kdevprojectmanager_part.cpp \
	kdevprojectmanager.cpp \
	kdevprojectfilter.cpp \
	kdevprojectmanagerdelegate.cpp \
	importprojectjob.cpp \
	importsourcefile_job.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevprojectmanager.desktop

rcdir = $(kde_datadir)/kdevprojectmanager
rc_DATA = kdevprojectmanager.rc

DOXYGEN_EMPTY = YES
include ../../Doxyfile.am
