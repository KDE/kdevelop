project(bazaar)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9527)

set(kdevbazaar_SRCS
    diffjob.cpp
    bzrannotatejob.cpp
    bazaarplugin.cpp
    bazaarpluginmetadata.cpp
    bazaarutils.cpp
    copyjob.cpp
)

add_library(kdevbazaar MODULE ${kdevbazaar_SRCS})
target_link_libraries(kdevbazaar
    ${KDEVPLATFORM_VCS_LIBRARIES}
)

install(TARGETS kdevbazaar DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})

configure_file(kdevbazaar.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevbazaar.desktop)
kcoreaddons_desktop_to_json(kdevbazaar ${CMAKE_CURRENT_BINARY_DIR}/kdevbazaar.desktop)

add_subdirectory(icons)
add_subdirectory(tests)
