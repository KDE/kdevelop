# Here resides the ld linker plugin.

INCLUDES = 	$(all_includes) \
					-I$(top_srcdir)/kdevelop/lib/Build/KDevLinker \
					-I$(top_srcdir)/kdevelop/lib/ComponentManager

lib_LTLIBRARIES = libKDevLinkerLd.la
libKDevLinkerLd_la_SOURCES = flagboxes.cpp LdLinkerOptionsWidgetBase.ui LdLinkerOptionsWidget.cpp \
																 KDevLinkerLd.cpp main.cpp
libKDevLinkerLd_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libKDevLinkerLd_la_LIBADD  = $(top_builddir)/kdevelop/lib/libkdevelop.la

noinst_HEADERS = LdLinkerOptionsWidgetBase.h LdLinkerOptionsWidget.h main.h \
																KDevLinkerLd.h flagboxes.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = KDevelopLdLinker.desktop
