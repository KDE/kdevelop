# Here resides the qextmdi main frame view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib -I$(top_srcdir)/kdevelop/widgets/qextmdi $(all_includes)

lib_LTLIBRARIES = libkdevqextmdimainfrmview.la
libkdevqextmdimainfrmview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevqextmdimainfrmview_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la $(top_builddir)/kdevelop/widgets/qextmdi/libkdevqextmdi.la $(LIB_KDEUI)

libkdevqextmdimainfrmview_la_SOURCES = main.cpp mdimainfrmcomponent.cpp mdiframe.cpp

noinst_HEADERS = main.h mdimainfrmcomponent.h mdiframe.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevmdimainfrmcomponent.desktop

rcdir = $(kde_datadir)/kdevqextmdimainfrmview
rc_DATA = kdevmdimainfrmcomponent.rc
