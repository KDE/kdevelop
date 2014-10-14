
set(kdevopenwith_PART_SRCS
    openwithplugin.cpp
)

add_library(kdevopenwith MODULE ${kdevopenwith_PART_SRCS})
target_link_libraries(kdevopenwith KDev::Interfaces KDev::Project KDev::Util)

install(TARGETS kdevopenwith DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})

########### install files ###############

configure_file(kdevopenwith.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevopenwith.desktop)
kcoreaddons_desktop_to_json(kdevopenwith ${CMAKE_CURRENT_BINARY_DIR}/kdevopenwith.desktop)
install( FILES iopenwith.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/openwith COMPONENT Devel)
