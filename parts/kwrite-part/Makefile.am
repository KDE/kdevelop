INCLUDES = $(all_includes) -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkwriteeditorpart.la

libkwriteeditorpart_la_SOURCES = kwrite_part.cpp kwrite_factory.cpp document_impl.cpp clipboard_iface_impl.cpp cursor_iface_impl.cpp undo_iface_impl.cpp documents_iface_impl.cpp

libkwriteeditorpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkwriteeditorpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la $(top_builddir)/lib/kwrite/libkdevkwrite.la -lktexteditor -lkspell

partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kwriteeditor_part.desktop

partrcdir   = $(kde_datadir)/kwriteeditor_part
partrc_DATA = kwriteeditor_part.rc
