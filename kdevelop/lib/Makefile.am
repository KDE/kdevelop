# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files


INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0 
libkdevelop_la_LIBADD = $(top_srcdir)/kdevelop/lib/general/libkdevgeneral.la \
			$(top_srcdir)/kdevelop/lib/sourceinfo/libkdevsourceinfo.la \
			$(top_srcdir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
			$(top_srcdir)/kdevelop/lib/kidetexteditor/libkidetexteditor.la \
			$(LIB_KPARTS) $(LIB_KFILE)
libkdevelop_la_SOURCES = dummy.cpp


SUBDIRS = servicetypes general sourceinfo projectmanagement kidetexteditor

