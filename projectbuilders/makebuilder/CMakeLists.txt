project(makebuilder)


########### next target ###############

set(kdevmakebuilder_SRCS
    makebuilder.cpp
    makejob.cpp
    makebuilderpreferences.cpp
)
ki18n_wrap_ui(kdevmakebuilder_SRCS makeconfig.ui)
kconfig_add_kcfg_files( kdevmakebuilder_SRCS makebuilderconfig.kcfgc )

add_library(kdevmakebuilder MODULE ${kdevmakebuilder_SRCS})
target_link_libraries(kdevmakebuilder
        KF5::TextEditor
        KDev::Interfaces
        KDev::Project
        KDev::OutputView
        KDev::Util
)


########### install files ###############
install(TARGETS kdevmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )
install(FILES imakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/make COMPONENT Devel)
configure_file(kdevmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevmakebuilder.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# install( FILES kdevmakebuilder.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevmakebuilder )
