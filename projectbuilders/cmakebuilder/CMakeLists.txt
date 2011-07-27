project(cmakebuilder)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9032 )
include_directories(
    ${makebuilder_SOURCE_DIR}
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake
    )


########### next target ###############

set(cmakebuilder_LIB_SRCS
    cmakebuilder.cpp
    cmakejob.cpp
    configureandbuildjob.cpp
)

kde4_add_plugin(kdevcmakebuilder ${cmakebuilder_LIB_SRCS})
target_link_libraries(
    kdevcmakebuilder
    kdev4cmakecommon
        ${KDE4_KIO_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

install(TARGETS kdevcmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kdevcmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevcmakebuilder )

