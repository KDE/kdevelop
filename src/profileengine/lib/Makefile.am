INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/external_interfaces \
	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
	$(KDEVMDI_INCLUDES) -I$(top_srcdir)/lib/structure $(all_includes)
METASOURCES = AUTO
libprofileengine_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libprofileengine.la
noinst_HEADERS = profileengine.h profile.h
libprofileengine_la_SOURCES = profileengine.cpp profile.cpp
