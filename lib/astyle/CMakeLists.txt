SET(astyle_STAT_SRCS
ASBeautifier.cpp
ASFormatter.cpp
ASResource.cpp
)

ADD_LIBRARY(astyle STATIC ${astyle_STAT_SRCS})



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#noinst_LTLIBRARIES = libastyle.la
#libastyle_la_LDFLAGS = $(all_libraries)
#libastyle_la_SOURCES = ASBeautifier.cpp ASFormatter.cpp ASResource.cpp

#######################
#previous CMakeLists.txt contents follow:

#
#ADD_LIBRARY(astyle STATIC  ASBeautifier.cpp ASFormatter.cpp ASResource.cpp)
