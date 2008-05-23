
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9032 )
include_directories(
    ${CMAKE_SOURCE_DIR}/buildtools/builders/makebuilder
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/outputview
    ${KDEVPLATFORM_INCLUDE_DIR}/util
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    )


########### next target ###############

set(cmakebuilder_LIB_SRCS
    cmakebuilder.cpp
)


kde4_add_plugin(kdevcmakebuilder ${cmakebuilder_LIB_SRCS})
target_link_libraries(
    kdevcmakebuilder
        ${KDE4_KDEUI_LIBS}
        ${QT_QTDESIGNER_LIBRARY}
        ${KDEVPLATFORM_INTERFACES_LIBRARY}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
        ${KDEVPLATFORM_UTIL_LIBRARY}
        ${KDEVPLATFORM_PROJECT_LIBRARY}
)

install(TARGETS kdevcmakebuilder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kdevcmakebuilder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES icmakebuilder.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cmake )
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevcmakebuilder )

