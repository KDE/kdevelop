# Detect Clang libraries
#
# Defines the following variables:
#  CLANG_CLANG_LIB             - LibClang library
#  CLANG_CLANGFRONTEND_LIB     - Clang Frontent Library
#  CLANG_CLANGDRIVER_LIB       - Clang Driver Library
#  ...
#
# Uses the same include and library paths detected by FindLLVM.cmake
#
# See http://clang.llvm.org/docs/InternalsManual.html for full list of libraries

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

find_package(LLVM ${Clang_FIND_VERSION} REQUIRED)

if (NOT LLVM_INCLUDE_DIR OR NOT LLVM_LIBRARY_DIR)
  message(FATAL_ERROR "Could not detect LLVM")
else()

macro(FIND_AND_ADD_CLANG_LIB _libname_)
  string(TOUPPER ${_libname_} _prettylibname_)
  find_library(CLANG_${_prettylibname_}_LIB ${_libname_} ${LLVM_LIBRARY_DIR} ${CLANG_LIB_DIR})
  if(CLANG_${_prettylibname_}_LIB)
    set(CLANG_LIBS ${CLANG_LIBS} ${CLANG_${_prettylibname_}_LIB})
  endif()
endmacro(FIND_AND_ADD_CLANG_LIB)

FIND_AND_ADD_CLANG_LIB(clang) # LibClang: high-level C interface
FIND_AND_ADD_CLANG_LIB(clangFrontend)
FIND_AND_ADD_CLANG_LIB(clangDriver)
FIND_AND_ADD_CLANG_LIB(clangCodeGen)
FIND_AND_ADD_CLANG_LIB(clangSema)
FIND_AND_ADD_CLANG_LIB(clangChecker)
FIND_AND_ADD_CLANG_LIB(clangAnalysis)
FIND_AND_ADD_CLANG_LIB(clangRewrite)
FIND_AND_ADD_CLANG_LIB(clangAST)
FIND_AND_ADD_CLANG_LIB(clangParse)
FIND_AND_ADD_CLANG_LIB(clangLex)
FIND_AND_ADD_CLANG_LIB(clangBasic)
FIND_AND_ADD_CLANG_LIB(clangARCMigrate)
FIND_AND_ADD_CLANG_LIB(clangEdit)
FIND_AND_ADD_CLANG_LIB(clangFrontendTool)
FIND_AND_ADD_CLANG_LIB(clangRewrite)
FIND_AND_ADD_CLANG_LIB(clangSerialization)
FIND_AND_ADD_CLANG_LIB(clangTooling)
FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCheckers)
FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerCore)
FIND_AND_ADD_CLANG_LIB(clangStaticAnalyzerFrontend)
FIND_AND_ADD_CLANG_LIB(clangSema)
FIND_AND_ADD_CLANG_LIB(clangRewriteCore)

if(CLANG_LIBS)
  set(CLANG_FOUND TRUE)
endif()

if(CLANG_FOUND)
  message(STATUS "Found Clang: ${LLVM_INCLUDE_DIR}")
  message(STATUS "  Libraries: ${CLANG_LIBS}")
else()
  if(Clang_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Clang")
  endif()
endif()

endif()
