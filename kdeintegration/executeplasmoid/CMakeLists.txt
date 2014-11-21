project(executeplasmoid)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########### next target ###############

set(kdevexecuteplasmoid_PART_UIS
    plasmoidexecutionconfig.ui
)

set(kdevexecuteplasmoid_PART_SRCS
    executeplasmoidplugin.cpp
    plasmoidexecutionconfig.cpp
    plasmoidexecutionjob.cpp
)

ki18n_wrap_ui( kdevexecuteplasmoid_PART_SRCS ${kdevexecuteplasmoid_PART_UIS} )
add_library(kdevexecuteplasmoid MODULE ${kdevexecuteplasmoid_PART_SRCS}  )
target_link_libraries(kdevexecuteplasmoid
    KDev::Language
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView)

install(TARGETS kdevexecuteplasmoid DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})


########### install files ###############
configure_file(kdevexecuteplasmoid.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevexecuteplasmoid.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevexecuteplasmoid.desktop DESTINATION ${SERVICES_INSTALL_DIR})
