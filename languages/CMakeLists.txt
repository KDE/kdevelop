ecm_optional_add_subdirectory(plugins)
ecm_optional_add_subdirectory(qmljs)

# TODO: For now, enable legacy support on Windows (kdev-clang is not compilable on latest MSVC)
# Enable clang by default on all other platforms
option(LEGACY_CPP_SUPPORT "Enable legacy C++ backend" ${WIN32})

if(LEGACY_CPP_SUPPORT)
    ecm_optional_add_subdirectory(cpp)
else()
    ecm_optional_add_subdirectory(clang)
endif()
