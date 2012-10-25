{% load kdev_filters %}
{{ license|lines_prepend:"# " }}


# - Try to find {{ name }}
# Once done this will define
#  {{ name|upper }}_FOUND - System has {{ name }}
#  {{ name|upper }}_INCLUDE_DIRS - The {{ name }} include directories
#  {{ name|upper }}_LIBRARIES - The libraries needed to use {{ name }}
#  {{ name|upper }}_DEFINITIONS - Compiler switches required for using {{ name }}


find_package(PkgConfig)
pkg_check_modules(PC_{{ name|upper }} QUIET {{ pc_package }})
set({{ name|upper }}_DEFINITIONS ${PC_{{ name|upper }}_CFLAGS_OTHER})


find_path({{ name|upper }}_INCLUDE_DIR {{ include_name }}
          HINTS ${PC_{{ name|upper }}_INCLUDEDIR} ${PC_{{ name|upper }}_INCLUDE_DIRS}
          PATH_SUFFIXES {{ name|lower}} )


find_library({{ name|upper }}_LIBRARY NAMES {{ library_name }}
             HINTS ${PC_{{ name|upper }}_LIBDIR} ${PC_{{ name|upper }}_LIBRARY_DIRS} )


set({{ name|upper }}_LIBRARIES ${% templatetag openbrace %}{{ name|upper }}_LIBRARY{% templatetag closebrace %} )
set({{ name|upper }}_INCLUDE_DIRS ${% templatetag openbrace %}{{ name|upper }}_INCLUDE_DIR{% templatetag closebrace %} )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set {{ name|upper }}_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args({{ name }}  DEFAULT_MSG
                                  {{ name|upper }}_LIBRARY {{ name|upper }}_INCLUDE_DIR)

mark_as_advanced({{ name|upper }}_INCLUDE_DIR {{ name|upper }}_LIBRARY )
