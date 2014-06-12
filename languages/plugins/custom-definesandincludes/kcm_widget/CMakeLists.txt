include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR} ../)

set( kcm_customdefinesandincludes_SRCS
        projectpathsmodel.cpp
        definesmodel.cpp
        includesmodel.cpp
        includeswidget.cpp
        defineswidget.cpp
        projectpathswidget.cpp
        kcm_customdefinesandincludes.cpp
        ../debugarea.cpp
        compilersmodel.cpp
        compilerswidget.cpp
   )

kde4_add_ui_files( kcm_customdefinesandincludes_SRCS
        batchedit.ui
        includeswidget.ui
        defineswidget.ui
        projectpathswidget.ui
        compilerswidget.ui )
kde4_add_kcfg_files( kcm_customdefinesandincludes_SRCS
        customdefinesandincludes.kcfgc )
kde4_add_plugin( kcm_kdevcustomdefinesandincludes
        ${kcm_customdefinesandincludes_SRCS} )

target_link_libraries( kcm_kdevcustomdefinesandincludes LINK_PRIVATE
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
        kdev4includesdefinessettings )

install( TARGETS kcm_kdevcustomdefinesandincludes
         DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kcm_kdevcustomdefinesandincludes.desktop
         DESTINATION ${SERVICES_INSTALL_DIR} )
