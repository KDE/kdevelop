SUBDIRS = gcc cpp
INCLUDES = $(all_includes) \
					 -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler/gcc \
					 -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler/cpp
lib_LTLIBRARIES = libKDevCompiler.la
libKDevCompiler_la_SOURCES = KDevCompiler.cpp
libKDevCompiler_la_LIBADD = $(top_builddir)/kdevelop/lib/projectmanagement/KDevCompiler/gcc/libKDevCompilerGcc.la \
														$(top_builddir)/kdevelop/lib/projectmanagement/KDevCompiler/cpp/libKDevCompilerCpp.la