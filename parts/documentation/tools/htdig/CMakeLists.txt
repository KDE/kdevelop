ADD_EXECUTABLE(kdevelop-htdig htdigindex.cpp)



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#METASOURCES = AUTO
#bin_PROGRAMS =  kdevelop-htdig
#kdevelop_htdig_LDFLAGS = $(all_libraries) $(LIB_KIO) $(LIB_KDEUI)
#kdevelop_htdig_SOURCES = htdigindex.cpp
#noinst_HEADERS =  htdigindex.h

#######################
#previous CMakeLists.txt contents follow:

#ADD_EXECUTABLE(kdevelop-htdig ${kdevelop-htdig_SRCS})
#
