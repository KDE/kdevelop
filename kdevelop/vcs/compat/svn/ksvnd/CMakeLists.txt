

#original Makefile.am contents follow:

##INCLUDES= -I$(top_srcdir)/kwallet/client $(all_includes)
#INCLUDES= $(all_includes)
#
#kde_module_LTLIBRARIES = kded_ksvnd.la
#
#kded_ksvnd_la_SOURCES = commitdlg.ui ksvnd.cpp ksvnd.skel 
#kded_ksvnd_la_METASOURCES = AUTO
#kded_ksvnd_la_LDFLAGS = $(all_libraries) -module -avoid-version
##kded_ksvnd_la_LIBADD = $(top_builddir)/kwallet/client/libkwalletclient.la $(LIB_KIO)
#kded_ksvnd_la_LIBADD = $(LIB_KIO)
#
#kded_DATA = ksvnd.desktop
#kdeddir = $(kde_servicesdir)/kded
