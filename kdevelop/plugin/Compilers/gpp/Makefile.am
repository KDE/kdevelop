# Here resides the Gpp compiler plugin.

INCLUDES = 	$(all_includes) \
					-I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler \
					-I$(top_srcdir)/kdevelop/lib/ComponentManager

lib_LTLIBRARIES = libKDevCompilerGpp.la
libKDevCompilerGpp_la_SOURCES = flagboxes.cpp GppCompilerOptionsWidgetBase.ui GppCompilerOptionsWidget.cpp \
																 KDevCompilerGpp.cpp main.cpp
libKDevCompilerGpp_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libKDevCompilerGpp_la_LIBADD  = $(top_builddir)/kdevelop/lib/libkdevelop.la

noinst_HEADERS = GppCompilerOptionsWidgetBase.h GppCompilerOptionsWidget.h main.h \
																KDevCompilerGpp.h flagboxes.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = KDevelopGppCompiler.desktop
