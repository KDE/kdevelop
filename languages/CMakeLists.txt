ecm_optional_add_subdirectory(plugins)
ecm_optional_add_subdirectory(qmljs)

find_package(Clang 3.8)
set(clangSearchHint "")
if (NOT CLANG_FOUND)
    set(clangSearchHint "Please install a package providing libclang. Either pass -DLLVM_ROOT=/path/to/llvm-prefix or install the 'llvm-config' command-line utility for auto-detection.")
endif()
set_package_properties(Clang PROPERTIES
    DESCRIPTION "Clang libraries from the LLVM project. ${clangSearchHint}"
    PURPOSE "Used for KDevelop's C++/C support plugin."
    TYPE REQUIRED
)

if (CLANG_FOUND)
    if (NOT CLANG_LIBCLANG_LIB)
        message(FATAL_ERROR "Could not find the Clang C library: libclang")
    endif()

    ecm_optional_add_subdirectory(clang)
endif()
