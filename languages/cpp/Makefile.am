##
## Do NOT remove the comments that start with "kdevelop:"
## They are actually directives to the kdevelop plugin system
##
## The include_xxxx variables are controlled by configure.in.in
##

SUBDIRS = parser app_templates

AM_CPPFLAGS = $(all_includes) -I$(top_srcdir)/lib/interfaces

lib_LTLIBRARIES = libkdevcpplanguagesupport.la
libkdevcpplanguagesupport_la_SOURCES = cpplanguagesupport.cpp cppsupportfactory.cpp
libkdevcpplanguagesupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcpplanguagesupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop

#rcdir = $(kde_datadir)/kdevelop/cpp
#rc_DATA = kdevcppsupport.rc

