INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)
METASOURCES = AUTO
libdesignerintegration_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libdesignerintegration.la
libdesignerintegration_la_SOURCES = implementationwidgetbase.ui \
	implementationwidget.cpp qtdesignerintegration.cpp
noinst_HEADERS = implementationwidget.h qtdesignerintegration.h
