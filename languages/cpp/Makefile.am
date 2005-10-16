##
## Do NOT remove the comments that start with "kdevelop:"
## They are actually directives to the kdevelop plugin system
##
## The include_xxxx variables are controlled by configure.in.in
##

SUBDIRS = parser app_templates

AM_CPPFLAGS = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/weaver \
	$(all_includes)

kde_module_LTLIBRARIES = libkdevcpplanguagesupport.la
libkdevcpplanguagesupport_la_SOURCES = cpplanguagesupport.cpp \
		cppsupportfactory.cpp backgroundparser.cpp parsejob.cpp preprocessjob.cpp
libkdevcpplanguagesupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
libkdevcpplanguagesupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la parser/libkdevcppparser.la

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop

#rcdir = $(kde_datadir)/kdevelop/cpp
#rc_DATA = kdevcppsupport.rc

noinst_HEADERS = backgroundparser.h parsejob.h preprocessjob.h

