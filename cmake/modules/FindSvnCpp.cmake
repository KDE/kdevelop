# - Try to find svncpp libraries
#
#  This macro uses the following variables as preference for seraching the
#  headers and includes:
#  SVNCPP_PATH - root directory where svncpp is installed (ususally /usr)
#
#  The variables set by this macro are:
#  SVNCPP_FOUND - system has svncpp libraries
#  SVNCPP_INCLUDE_DIR - the include directory to link to svncpp
#  SVNCPP_LIBRARY - The library needed to link to svncpp

if(SVNCPP_INCLUDE_DIR AND SVNCPP_LIBRARY)
    # Already in cache, be silent
    set(SvnCpp_FIND_QUIETLY TRUE)
endif(SVNCPP_INCLUDE_DIR AND SVNCPP_LIBRARY)

if( SVNCPP_PATH )
    set(_SVNCPP_EXTRA_LIB_PATHS ${SVNCPP_PATH}/lib  )
    set(_SVNCPP_EXTRA_INC_PATHS ${SVNCPP_PATH}/include  )
endif( SVNCPP_PATH )

if(WIN32)
    set(_SVNCPP_EXTRA_LIB_PATHS ${_SVNCPP_EXTRA_LIB_PATHS} "$ENV{ProgramFiles}/svncpp/lib  )
    set(_SVNCPP_EXTRA_INC_PATHS ${_SVNCPP_EXTRA_LIB_PATHS} "$ENV{ProgramFiles}/svncpp/include  )
endif(WIN32)

FIND_PATH(SVNCPP_INCLUDE_DIR svncpp/client.hpp
    PATHS
        ${_SVNCPP_EXTRA_INC_PATHS}
)
FIND_LIBRARY(SVNCPP_LIBRARY svncpp
    PATHS
        ${_SVNCPP_EXTRA_LIB_PATHS}
)

##############################
# Setup the result variables #
##############################

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( SvnCpp DEFAULT_MSG SVNCPP_LIBRARY SVNCPP_INCLUDE_DIR )

set(SVNCPP_LIBRARY ${SVNCPP_LIBRARY} CACHE STRING "svncpp library")
set(SVNCPP_INCLUDE_DIR ${SVNCPP_INCLUDE_DIR} CACHE STRING "svncpp include directories")

mark_as_advanced(
  SVNCPP_LIBRARY
  SVNCPP_INCLUDE_DIR
)


#kate: space-indent on; indent-width 2; tab-width: 2; replace-tabs on; auto-insert-doxygen on
