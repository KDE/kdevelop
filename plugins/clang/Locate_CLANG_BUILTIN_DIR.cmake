set(CLANG_VERSION "${LLVM_PACKAGE_VERSION}")
set(CLANG_VERSION_MAJOR "${LLVM_VERSION_MAJOR}")

# Since https://github.com/llvm/llvm-project/commit/e1b88c8a09be25b86b13f98755a9bd744b4dbf14
# Clang's resource directory includes only the major version.
if (CLANG_VERSION_MAJOR GREATER_EQUAL 16)
    set(CLANG_VERSION_SUBDIR "${CLANG_VERSION_MAJOR}")
else()
    # Git version of Clang ends with the "git" suffix, e.g. "14.0.0git", but
    # installs builtin headers into a subdirectory without the suffix, e.g. "14.0.0".
    string(REPLACE "git" "" CLANG_VERSION_SUBDIR "${CLANG_VERSION}")
endif()

message(STATUS "  LLVM library directories:   ${LLVM_LIBRARY_DIRS}")
message(STATUS "  Clang include directories:  ${CLANG_INCLUDE_DIRS}")

find_path(CLANG_BUILTIN_DIR
    # cpuid.h because it is defined in ClangSupport constructor as valid clang builtin dir indicator
    NAMES "cpuid.h"
    PATHS   "${LLVM_LIBRARY_DIRS}"
            "${CLANG_INCLUDE_DIRS}"
    PATH_SUFFIXES   "clang/${CLANG_VERSION_SUBDIR}/include"
                    "../lib/clang/${CLANG_VERSION_SUBDIR}/include"
                    "../../../clang/${CLANG_VERSION_SUBDIR}/include"
                    "../../../lib/clang/${CLANG_VERSION_SUBDIR}/include"
    NO_DEFAULT_PATH
)

if (NOT CLANG_BUILTIN_DIR)
    message(FATAL_ERROR "Could not find Clang builtin include directory")
endif()
get_filename_component(CLANG_BUILTIN_DIR "${CLANG_BUILTIN_DIR}" ABSOLUTE)

message(STATUS "  Builtin include directory:  ${CLANG_BUILTIN_DIR}")
