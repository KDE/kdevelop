INCLUDES = $(all_includes)

lib_LTLIBRARIES = libKDevBuild.la
libKDevBuild_la_LIBADD = 	$(top_builddir)/kdevelop/lib/Build/KDevCompiler/libKDevCompiler.la \
													$(top_builddir)/kdevelop/lib/Build/KDevLinker/libKDevLinker.la \
													$(top_builddir)/kdevelop/lib/Build/KDevMake/libKDevMake.la
libKDevBuild_la_SOURCES = dummy.cpp

SUBDIRS = KDevCompiler KDevLinker KDevMake

