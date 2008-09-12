#

SUBDIRS = file-templates

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevfilecreate.la
libkdevfilecreate_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfilecreate_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevfilecreate_la_SOURCES = filecreate_part.cpp filecreate_listitem.cpp \
	filecreate_filetype.cpp filecreate_filedialog.cpp fcconfigwidgetbase.ui fcconfigwidget.cpp \
	fctypeeditbase.ui fctypeedit.cpp fctemplateeditbase.ui fctemplateedit.cpp \
	filecreate_newfile.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfilecreate.desktop

rcdir = $(kde_datadir)/kdevfilecreate
rc_DATA = kdevpart_filecreate.rc template-info.xml
