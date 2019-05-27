# - Find LibAStyle
# Find the Artistic Style library.
#
# This module defines
#  LibAStyle_FOUND - whether the astyle library was found
#  LibAStyle_LIBRARIES - the astyle library
#  LibAStyle_INCLUDE_DIR - the include path of the astyle library
#  LibAStyle_CFLAGS - the compiler flags needed when building with the astyle library

#=============================================================================
# Copyright 2018-2019 Pino Toscano <pino@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_library(LibAStyle_LIBRARIES
  NAMES astyle
)

find_path(LibAStyle_INCLUDE_DIR
  NAMES astyle.h
)

# astyle is built without RTTI
set(LibAStyle_CFLAGS "-fno-rtti")

if(LibAStyle_LIBRARIES AND LibAStyle_INCLUDE_DIR)
  include(CMakePushCheckState)
  include(CheckCXXSourceCompiles)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_FLAGS "${LibAStyle_CFLAGS}")
  set(CMAKE_REQUIRED_LIBRARIES "${LibAStyle_LIBRARIES}")
  set(CMAKE_REQUIRED_INCLUDES "${LibAStyle_INCLUDE_DIR}")
  check_cxx_source_compiles("
#include <astyle.h>
int main()
{
  astyle::ASPeekStream foo(nullptr);
  return 0;
}" LIBASTYLE_HAS_ASPEEKSTREAM)
  if(LIBASTYLE_HAS_ASPEEKSTREAM)
    set(_libastyle_version "3.1")
    message(STATUS "LibAStyle version autodetected as ${_libastyle_version} or greater")
  else()
    set(_libastyle_version "0")
  endif()
  cmake_pop_check_state()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibAStyle
  FOUND_VAR LibAStyle_FOUND
  REQUIRED_VARS LibAStyle_LIBRARIES LibAStyle_INCLUDE_DIR
  VERSION_VAR _libastyle_version
)

mark_as_advanced(LibAStyle_INCLUDE_DIR LibAStyle_LIBRARIES)

if(LibAStyle_FOUND)
  add_library(astylelib UNKNOWN IMPORTED)
  set_target_properties(astylelib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LibAStyle_INCLUDE_DIR}"
    IMPORTED_LOCATION "${LibAStyle_LIBRARIES}"
    INTERFACE_COMPILE_OPTIONS "${LibAStyle_CFLAGS}"
  )
endif()
