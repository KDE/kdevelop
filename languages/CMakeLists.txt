ecm_optional_add_subdirectory(plugins)
ecm_optional_add_subdirectory(qmljs)

option(LEGACY_CPP_SUPPORT "Enable legacy C++ backend" OFF)

if(LEGACY_CPP_SUPPORT)
    ecm_optional_add_subdirectory(cpp)
else()
    ecm_optional_add_subdirectory(clang)
endif()
