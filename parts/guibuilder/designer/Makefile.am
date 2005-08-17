# Here resides the designer part.

INCLUDES = \
  -I$(top_srcdir)/lib/interfaces \
  -I$(top_srcdir)/lib/projectmanager/interfaces \
  -I$(top_srcdir)/lib/util \
  $(all_includes)

kde_module_LTLIBRARIES = libkdevdesignerpart.la
libkdevdesignerpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdesignerpart_la_LIBADD = \
  $(top_builddir)/lib/libkdevelop.la \
  $(top_builddir)/lib/projectmanager/interfaces/libprojectmanager_interfaces.la \
  $(LIB_QTDESIGNER) $(LIB_KFILE) $(LIB_KPARTS)

libkdevdesignerpart_la_SOURCES = \
  kdevdesigner_part.cpp

METASOURCES = AUTO

#partrcdir = $(kde_datadir)/kdevdesignerpart
#partrc_DATA = kdevdesigner_part.rc

partdesktopdir = $(kde_servicesdir)
partdesktop_DATA = kdevdesigner_part.desktop
