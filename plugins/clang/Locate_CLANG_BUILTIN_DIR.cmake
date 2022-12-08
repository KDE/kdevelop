set(CLANG_VERSION "${LLVM_PACKAGE_VERSION}")

# svn version of clang has a svn suffix "8.0.0svn" but installs the header in "8.0.0", without the suffix
string(REPLACE "svn" "" CLANG_VERSION_CLEAN "${CLANG_VERSION}")
# ditto for git
string(REPLACE "git" "" CLANG_VERSION_CLEAN "${CLANG_VERSION}")

message(STATUS "  LLVM library directories:   ${LLVM_LIBRARY_DIRS}")
message(STATUS "  Clang include directories:  ${CLANG_INCLUDE_DIRS}")

find_path(CLANG_BUILTIN_DIR
    # cpuid.h because it is defined in ClangSupport constructor as valid clang builtin dir indicator
    NAMES "cpuid.h"
    PATHS   "${LLVM_LIBRARY_DIRS}"
            "${CLANG_INCLUDE_DIRS}"
    PATH_SUFFIXES   "clang/${CLANG_VERSION}/include"
                    "../../../clang/${CLANG_VERSION}/include"
                    "clang/${CLANG_VERSION_CLEAN}/include"
                    "../../../clang/${CLANG_VERSION_CLEAN}/include"
    NO_DEFAULT_PATH
)

if (NOT CLANG_BUILTIN_DIR)
    message(FATAL_ERROR "Could not find Clang builtin include directory")
endif()
get_filename_component(CLANG_BUILTIN_DIR "${CLANG_BUILTIN_DIR}" ABSOLUTE)

message(STATUS "  Builtin include directory:  ${CLANG_BUILTIN_DIR}")
