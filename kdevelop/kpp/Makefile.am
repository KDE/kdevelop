noinst_LIBRARIES = libkpp.a
 
libkpp_a_SOURCES = projectView.ui prefView.ui kpp.cpp cspec.cpp cproject.cpp
#libkpp_a_LIBADD   =  $(LIB_KFILE)

datadir = $(kde_datadir)/kdevelop/
data_DATA = template.spec buildrpm.sh

EXTRA_DIST = cproject.cpp cproject.h cspec.cpp cspec.h kpp.cpp kpp.h prefView.ui projectView.ui

INCLUDES= $(all_includes)

libkpp_a_METASOURCES = AUTO

# the library search path.
libkpp_a_LIBFLAGS = $(all_libraries) $(KDE_RPATH)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kpp.pot
