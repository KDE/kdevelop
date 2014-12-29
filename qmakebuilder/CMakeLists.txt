add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9039 )

set(qmakebuilder_SRCS
    qmakebuilder.cpp
    qmakebuilderpreferences.cpp
    ../qmakebuilddirchooser.cpp
    ../qmakebuilddirchooserdialog.cpp
    ../qmakeconfig.cpp
    qmakejob.cpp
    ../qmakeconfig.cpp
)
set(qmakebuilder_UI
    qmakeconfig.ui
    ../qmakebuilddirchooser.ui
)
qt5_wrap_ui(qmakebuilder_SRCS ${qmakebuilder_UI})
kconfig_add_kcfg_files(qmakebuilder_SRCS qmakebuilderconfig.kcfgc)
add_library(kdevqmakebuilder MODULE ${qmakebuilder_SRCS})
target_link_libraries(kdevqmakebuilder
    ${QT_QTDESIGNER_LIBRARY}
    KF5::KDELibs4Support
    KDev::Interfaces
    KDev::OutputView
    KDev::Util
    KDev::Project
)

install(TARGETS kdevqmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(kdevqmakebuilder.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevqmakebuilder.desktop)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevqmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES iqmakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/qmake COMPONENT Devel)
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevqmakebuilder )
