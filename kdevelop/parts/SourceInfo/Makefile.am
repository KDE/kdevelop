INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general \
													-I$(top_srcdir)/kdevelop/lib/ComponentManager \
													-I$(top_srcdir)/kdevelop/lib/ComponentManager/Components

lib_LTLIBRARIES = libkdevsourceinfo.la

libkdevsourceinfo_la_SOURCES = classstore.cpp parsedclass.cpp parsedmethod.cpp \
			       parsedattribute.cpp parsedargument.cpp parsedparent.cpp parsedsignalslot.cpp \
			       persistantclassstore.cpp parsedstruct.cpp classtreenode.cpp parsedcontainer.cpp \
			       parsedclasscontainer.cpp parseditem.cpp parsedscopecontainer.cpp main.cpp

libkdevsourceinfo_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevsourceinfo_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la
noinst_HEADERs = classstore.h parsedclass.h parsedmethod.h parsedattribute.h parsedargument.h parsedparent.h \
								 parsedsignalslot.h persistantclassstore.h parsedstruct.h classtreenode.h parseditem.h \
								 parsedcontainer.h parsedclasscontainer.h parsedscopecontainer.h programmingbycontract.h main.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevsourceinfo.desktop
