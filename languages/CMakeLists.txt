ecm_optional_add_subdirectory(plugins)
ecm_optional_add_subdirectory(qmljs)

find_package(Clang 3.8)
set_package_properties(Clang PROPERTIES
    PURPOSE "Clang libraries. Used for KDevelop's C++/C support plugin. Please install a package providing libclang."
    TYPE REQUIRED
)

if (CLANG_FOUND)
    if (NOT CLANG_LIBCLANG_LIB)
        message(FATAL_ERROR "Could not find the Clang C library: libclang")
    endif()

    ecm_optional_add_subdirectory(clang)
endif()
