INCLUDES = $(all_includes) -I$(top_srcdir)/lib/interfaces
METASOURCES = AUTO

kde_module_LTLIBRARIES = libneditpart.la

libneditpart_la_SOURCES = nedit_part.cpp nedit_factory.cpp document_impl.cpp cursor_iface_impl.cpp

libneditpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libneditpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la

partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = nedit_part.desktop

partrcdir   = $(kde_datadir)/nedit_part
partrc_DATA = nedit_part.rc
