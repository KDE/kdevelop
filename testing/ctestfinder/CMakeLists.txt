project(ctestintegration)
#TODO: add_definitions( -DKDE_DEFAULT_DEBUG_AREA=90xx )

########### next target ###############

set(ctestintegration_LIB_SRCS
    ctestfinder.cpp
    ctestfindjob.cpp
    ctestsuite.cpp
    ctestrunjob.cpp
    ctestoutputmodel.cpp
    qttestdelegate.cpp
)

kde4_add_plugin(kdevctestintegration ${ctestintegration_LIB_SRCS})
target_link_libraries(
    kdevctestintegration
        ${KDE4_KIO_LIBS}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

install(TARGETS kdevctestintegration DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES ictestprovider.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/ctest COMPONENT Devel)
install( FILES kdevctestintegration.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
