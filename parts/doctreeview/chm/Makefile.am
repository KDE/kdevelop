## Makefile.am of kdebase/kioslave/chm

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kio_chm.la libkchmpart.la

kio_chm_la_SOURCES = decompress.cpp chmfile.cpp chm.cpp
kio_chm_la_LIBADD = $(LIB_KIO)
kio_chm_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

libkchmpart_la_SOURCES = kchmpart.cpp
libkchmpart_la_LIBADD = -lkhtml $(LIB_KPARTS)
libkchmpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

noinst_HEADERS = chm.h chmfile.h decompress.h kchmpart.h

kdelnk_DATA = chm.protocol kchmpart.desktop
kdelnkdir = $(kde_servicesdir)

METASOURCES = AUTO

