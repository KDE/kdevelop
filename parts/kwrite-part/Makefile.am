SUBDIRS = kwrite

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkwriteeditorpart.la

libkwriteeditorpart_la_SOURCES = kwrite_part.cpp kwrite_factory.cpp document_impl.cpp clipboard_iface_impl.cpp cursor_iface_impl.cpp undo_iface_impl.cpp edit_iface_impl.cpp status_iface_impl.cpp debug_iface_impl.cpp codecompletion_iface_impl.cpp codecompletion_arghint.cpp

libkwriteeditorpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkwriteeditorpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la kwrite/libkdevkwrite.la -lktexteditor -lkspell

partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kwriteeditor_part.desktop

partrcdir   = $(kde_datadir)/kwriteeditor_part
partrc_DATA = kwriteeditor_part.rc
