project(qmakebuilder)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9039 )

include_directories( ${qmakebuilder_BINARY_DIR} )

########### next target ###############

set(qmakebuilder_LIB_SRCS
    qmakebuilder.cpp
    qmakejob.cpp
    ../qmakeconfig.cpp
)

add_library(kdevqmakebuilder MODULE ${qmakebuilder_LIB_SRCS})
target_link_libraries(kdevqmakebuilder
    ${QT_QTDESIGNER_LIBRARY}
    KF5::KDELibs4Support
    KDev::Interfaces
    KDev::OutputView
    KDev::Util
    KDev::Project
)

install(TARGETS kdevqmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############
set( qmakebuilder_cfg_SRCS
    qmakebuilderpreferences.cpp
    ../qmakebuilddirchooser.cpp
    ../qmakebuilddirchooserdialog.cpp
    ../qmakeconfig.cpp
)

set( qmakebuilder_cfg_UI
    qmakeconfig.ui
    ../qmakebuilddirchooser.ui
)

qt5_wrap_ui(qmakebuilder_cfg_SRCS ${qmakebuilder_cfg_UI})
kconfig_add_kcfg_files(qmakebuilder_cfg_SRCS qmakebuilderconfig.kcfgc)
add_library(kcm_kdev_qmakebuilder MODULE ${qmakebuilder_cfg_SRCS})
target_link_libraries(kcm_kdev_qmakebuilder
    KF5::KIOWidgets
    KF5::KDELibs4Support
    KDev::Interfaces
    KDev::Project
    KDev::Util
)

########### install files ###############
configure_file(kdevqmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevqmakebuilder.desktop)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevqmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES iqmakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/qmake COMPONENT Devel)
install( TARGETS kcm_kdev_qmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_qmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevqmakebuilder )


