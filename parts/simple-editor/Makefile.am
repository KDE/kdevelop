INCLUDES = $(all_includes) -I$(top_srcdir)/lib/interfaces
METASOURCES = AUTO

kde_module_LTLIBRARIES = libeditortestpart.la

libeditortestpart_la_SOURCES = editortest_part.cpp editortest_factory.cpp document_impl.cpp clipboard_iface_impl.cpp undo_iface_impl.cpp cursor_iface_impl.cpp documents_iface_impl.cpp

libeditortestpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libeditortestpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la

partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = editortest_part.desktop

partrcdir   = $(kde_datadir)/editortest_part
partrc_DATA = editortest_part.rc
