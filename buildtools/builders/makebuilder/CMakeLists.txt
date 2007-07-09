
include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/outputview
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/util
    )


########### next target ###############

set(kdevmakebuilder_LIB_SRCS
    makebuilder.cpp
    makeoutputmodel.cpp
    outputfilters.cpp
    makeitem.cpp
)


kde4_add_plugin(kdevmakebuilder ${kdevmakebuilder_LIB_SRCS})
target_link_libraries(kdevmakebuilder
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KTEXTEDITOR_LIBS}
        ${QT_QTDESIGNER_LIBRARY}
        ${KDEVPLATFORM_INTERFACES_LIBRARY}
        ${KDEVPLATFORM_PROJECT_LIBRARY}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
        ${KDEVPLATFORM_UTIL_LIBRARY}
)

install(TARGETS kdevmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )


set( makebuilder_cfg_SRCS
    makebuilderpreferences.cpp
)

set( makebuilder_cfg_UI
    makeconfig.ui
)


kde4_add_ui_files( makebuilder_cfg_SRCS ${makebuilder_cfg_UI} )
kde4_add_kcfg_files( makebuilder_cfg_SRCS makebuilderconfig.kcfgc )
kde4_add_plugin( kcm_kdev_makebuilder ${makebuilder_cfg_SRCS} )
target_link_libraries( kcm_kdev_makebuilder ${KDE4_KUTILS_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_PROJECT_LIBRARY} )

install( TARGETS kcm_kdev_makebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES imakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/make)
install( FILES kcm_kdev_makebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
# install( FILES kdevmakebuilder.rc DESTINATION ${DATA_INSTALL_DIR}/kdevmakebuilder )

