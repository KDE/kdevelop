
# We need exceptions since they are used to report all MI access errors.
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

METASOURCES = AUTO
INCLUDES = $(all_includes)

lib_LTLIBRARIES = libgdbmi_parser.la
libgdbmi_parser_la_LDFLAGS = $(all_libraries)
libgdbmi_parser_la_SOURCES = gdbmi.cpp miparser.cpp milexer.cpp

