# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files


INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0 
libkdevelop_la_LIBADD = $(top_builddir)/kdevelop/lib/general/libkdevgeneral.la \
			$(top_builddir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
			$(top_builddir)/kdevelop/lib/kidetexteditor/libkidetexteditor.la \
			$(top_builddir)/kdevelop/lib/ComponentManager/libKDevComponentManager.la \
			$(top_builddir)/kdevelop/lib/Interfaces/libKDevInterfaces.la \
			$(LIB_KPARTS) $(LIB_KFILE)
libkdevelop_la_SOURCES = dummy.cpp


SUBDIRS = general projectmanagement kidetexteditor ComponentManager Interfaces

