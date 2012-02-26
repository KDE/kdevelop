project(ctestfinder)
#TODO: add_definitions( -DKDE_DEFAULT_DEBUG_AREA=90xx )

########### next target ###############

set(ctestfinder_LIB_SRCS
    ctestfinder.cpp
    ctestfindjob.cpp
    ctestsuite.cpp
    ctestrunjob.cpp
    ctestoutputmodel.cpp
    qttestdelegate.cpp
)

kde4_add_plugin(kdevctestfinder ${ctestfinder_LIB_SRCS})
target_link_libraries(
    kdevctestfinder
        ${KDE4_KIO_LIBS}
        ${KDEVPLATFORM_INTERFACES_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

install(TARGETS kdevctestfinder DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES ictestprovider.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/ctest COMPONENT Devel)
install( FILES kdevctestfinder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
