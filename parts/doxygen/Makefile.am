# Here resides the doxygen part

INCLUDES = -DDOXYWIZARD -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevdoxygen.la
libkdevdoxygen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdoxygen_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevdoxygen_la_SOURCES = doxygenpart.cpp \
			    doxygenconfigwidget.cpp \
			    input.cpp \
			    config.cpp \
			    version.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdoxygen.desktop

rcdir = $(kde_datadir)/kdevdoxygen
rc_DATA = kdevdoxygen.rc
