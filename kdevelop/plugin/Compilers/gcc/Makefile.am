# Here resides the gcc compiler plugin.

INCLUDES = 	$(all_includes) \
					-I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler \
					-I$(top_srcdir)/kdevelop/lib/ComponentManager

lib_LTLIBRARIES = libKDevCompilerGcc.la
libKDevCompilerGcc_la_SOURCES = flagboxes.cpp GccCompilerOptionsWidgetBase.ui GccCompilerOptionsWidget.cpp \
																KDevCompilerGcc.cpp main.cpp
libKDevCompilerGcc_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libKDevCompilerGcc_la_LIBADD  = $(top_builddir)/kdevelop/lib/libkdevelop.la

noinst_HEADERS = main.h GccCompilerOptionsWidgetBase.h GccCompilerOptionsWidget.h \
																KDevCompilerGcc.h flagboxes.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = KDevelopGccCompiler.desktop
