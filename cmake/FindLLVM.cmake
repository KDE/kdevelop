# Find the native LLVM includes and libraries
#
# Defines the following variables
#  LLVM_INCLUDE_DIR - where to find llvm include files
#  LLVM_LIBRARY_DIR - where to find llvm libs
#  LLVM_CFLAGS      - llvm compiler flags
#  LLVM_LFLAGS      - llvm linker flags
#  LLVM_MODULE_LIBS - list of llvm libs for working with modules.
#  LLVM_FOUND       - True if llvm found.
#  LLVM_VERSION     - Version string ("llvm-config --version")

#=============================================================================
# Copyright 2014 Kevin Funk <kevin@kfunk.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# find llvm-config, prefer the one with a version suffix, e.g. llvm-config-3.3
# note: on some distributions, only 'llvm-config' is shipped, so let's always try to fallback on that
find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config-${LLVM_FIND_VERSION} llvm-config DOC "llvm-config executable")

# other distributions don't ship llvm-config, but only some llvm-config-VERSION binary
# try to deduce installed LLVM version by looking up llvm-nm in PATH and *then* find llvm-config-VERSION via that
if (NOT LLVM_CONFIG_EXECUTABLE)
  find_program(_llvmNmExecutable llvm-nm)
  if (_llvmNmExecutable)
    execute_process(COMMAND ${_llvmNmExecutable} --version OUTPUT_VARIABLE _out)
    string(REGEX REPLACE ".*LLVM version ([^ \n]+).*" "\\1" _versionString "${_out}")
    find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config-${_versionString} DOC "llvm-config executable")
  endif()
endif()

if (LLVM_CONFIG_EXECUTABLE)
  message(STATUS "Found llvm-config: ${LLVM_CONFIG_EXECUTABLE}")
else()
  message(FATAL_ERROR "Could NOT find 'llvm-config' executable")
endif()

# verify that we've found the correct version of llvm-config
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
  OUTPUT_VARIABLE LLVM_VERSION)
if (NOT LLVM_VERSION)
  message(FATAL_ERROR "Failed to parse version from llvm-config")
endif()
if (LLVM_FIND_VERSION VERSION_GREATER LLVM_VERSION)
  message(FATAL_ERROR "LLVM version too old: ${LLVM_VERSION}")
endif()


execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
  OUTPUT_VARIABLE LLVM_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
  OUTPUT_VARIABLE LLVM_LIBRARY_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cppflags
  OUTPUT_VARIABLE LLVM_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
  OUTPUT_VARIABLE LLVM_LFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core bitreader asmparser analysis
  OUTPUT_VARIABLE LLVM_MODULE_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
