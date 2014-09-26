project(executescript)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9515 )

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########### next target ###############

set(kdevexecutescript_PART_UIS
    scriptappconfig.ui
)

set(kdevexecutescript_PART_SRCS
    executescriptplugin.cpp
    scriptappconfig.cpp
    scriptappjob.cpp
)

qt5_wrap_ui( kdevexecutescript_PART_SRCS ${kdevexecutescript_PART_UIS} )
add_library(kdevexecutescript MODULE ${kdevexecutescript_PART_SRCS})
target_link_libraries(kdevexecutescript
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView)

install(TARGETS kdevexecutescript DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})


########### install files ###############
install(FILES iexecutescriptplugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/executescript COMPONENT Devel)
configure_file(kdevexecutescript.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevexecutescript.desktop)
kcoreaddons_desktop_to_json(kdevexecutescript ${CMAKE_CURRENT_BINARY_DIR}/kdevexecutescript.desktop)
