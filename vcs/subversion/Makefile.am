INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/interfaces/extensions $(SVN_INCLUDE) $(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES = libkdevsubversion.la kio_kdevsvn.la kded_kdevsvnd.la

libkdevsubversion_la_SOURCES = subversion_core.cpp subversion_fileinfo.cpp \
	subversion_fileinfo.skel subversion_part.cpp subversion_widget.cpp subversiondiff.ui \
	subversionprojectwidget.ui svn_blamewidget.cpp svn_co.ui svn_commitdlgbase.ui \
	svn_fileselectdlg_commit.cpp svn_logviewoptiondlgbase.ui svn_logviewwidget.cpp
libkdevsubversion_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/extensions/libkdevextensions.la
libkdevsubversion_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

	
kio_kdevsvn_la_SOURCES = svn_kio.cpp
kio_kdevsvn_la_LIBADD = $(LIB_KIO)
kio_kdevsvn_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN) $(SVN_LIB)

kded_kdevsvnd_la_SOURCES = commitdlg.ui kdevsvnd.cpp kdevsvnd.skel kdevsvnd_widgets.cpp svnssltrustpromptbase.ui
kded_kdevsvnd_la_LIBADD = $(LIB_KIO) -lkdeinit_kded
kded_kdevsvnd_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)

kdeddir = $(kde_servicesdir)/kded
kded_DATA = kdevsvnd.desktop

servicedir = $(kde_servicesdir)
service_DATA = kdevsubversion.desktop

protocoldir = $(kde_servicesdir)
protocol_DATA = kdevsvn+file.protocol kdevsvn+http.protocol kdevsvn+https.protocol kdevsvn+ssh.protocol kdevsvn+svn.protocol


SUBDIRS = integrator
noinst_HEADERS = kdevsvnd.h svn_blamewidget.h svn_fileselectdlg_commit.h \
	svn_logviewwidget.h
