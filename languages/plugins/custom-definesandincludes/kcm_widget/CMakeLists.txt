include_directories(  ${KDEVPLATFORM_INCLUDE_DIR} ../)

set( kcm_customdefinesandincludes_SRCS
        projectpathsmodel.cpp
        definesmodel.cpp
        includesmodel.cpp
        includeswidget.cpp
        defineswidget.cpp
        projectpathswidget.cpp
        kcm_customdefinesandincludes.cpp
        ../debugarea.cpp
        ../compilerprovider/icompiler.cpp
        compilersmodel.cpp
        compilerswidget.cpp
   )

qt5_wrap_ui( kcm_customdefinesandincludes_SRCS
        batchedit.ui
        includeswidget.ui
        defineswidget.ui
        projectpathswidget.ui
        compilerswidget.ui )
kconfig_add_kcfg_files( kcm_customdefinesandincludes_SRCS
        customdefinesandincludes.kcfgc )
add_library( kcm_kdevcustomdefinesandincludes MODULE
        ${kcm_customdefinesandincludes_SRCS} )

target_link_libraries( kcm_kdevcustomdefinesandincludes LINK_PRIVATE
        KDev::Interfaces
        KDev::Project
        KDev::Util
        KDev::Language
        kdevdefinesandincludessettings )

install(TARGETS kcm_kdevcustomdefinesandincludes DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm)
configure_file(kcm_kdevcustomdefinesandincludes.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevcustomdefinesandincludes.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevcustomdefinesandincludes.desktop DESTINATION ${SERVICES_INSTALL_DIR})
