INCLUDES = $(all_includes) -I$(top_srcdir)/lib/interfaces
METASOURCES = AUTO

kde_module_LTLIBRARIES = libqeditorpart.la

libqeditorpart_la_SOURCES = \
	clipboard_iface_impl.cpp       paragdata.cpp \
	codecompletion_arghint.cpp     parenmatcher.cpp \
	codecompletion_iface_impl.cpp  qeditor.cpp \
	cursor_iface_impl.cpp	       qeditor_part.cpp \
	document_impl.cpp	       qsourcecolorizer.cpp \
	edit_iface_impl.cpp	       simple_indent.cpp \
	undo_iface_impl.cpp            cpp_colorizer.cpp \
	qeditor_factory.cpp            xml_colorizer.cpp \
	java_colorizer.cpp             qmake_colorizer.cpp \
	python_colorizer.cpp  


libqeditorpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libqeditorpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la

partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = qeditor_part.desktop

partrcdir   = $(kde_datadir)/qeditor_part
partrc_DATA = qeditor_part.rc
