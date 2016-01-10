ecm_optional_add_subdirectory(plugins)
ecm_optional_add_subdirectory(qmljs)

option(LEGACY_CPP_SUPPORT "Enable legacy C++ backend" OFF)

if(LEGACY_CPP_SUPPORT)
    ecm_optional_add_subdirectory(cpp)
else()
    find_package(Clang 3.5)
    set_package_properties(Clang PROPERTIES
        PURPOSE "Clang libraries. Used for KDevelop's C++/C support plugin"
        TYPE REQUIRED
    )

    if (CLANG_FOUND)
        if (NOT CLANG_LIBCLANG_LIB)
            message(FATAL_ERROR "Could not find the Clang C library: libclang")
        endif()

        ecm_optional_add_subdirectory(clang)
    endif()
endif()
