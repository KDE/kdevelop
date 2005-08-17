INCLUDES = -I$(top_srcdir)/lib/projectmanager/interfaces \
	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets/propeditor $(all_includes)

SUBDIRS = interfaces .

kde_module_LTLIBRARIES = libkdevprojectmanager.la
libkdevprojectmanager_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevprojectmanager_la_LIBADD = \
	$(top_builddir)/lib/projectmanager/interfaces/libprojectmanager_interfaces.la $(top_builddir)/lib/libkdevelop.la

libkdevprojectmanager_la_SOURCES = \
	kdevprojectmanager_part.cpp \
	kdevprojectmanager.cpp \
	kdevprojectmanager_model.cpp \
	importprojectjob.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevprojectmanager.desktop

rcdir = $(kde_datadir)/kdevprojectmanager
rc_DATA = \
    kdevprojectmanager.rc

DOXYGEN_EMPTY = YES
include ../../Doxyfile.am
