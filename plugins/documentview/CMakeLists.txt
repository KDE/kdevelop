
#add_subdirectory(settings)

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/util
)


########### next target ###############

set(kdevdocumentview_PART_SRCS
    kdevdocumentview.cpp
    kdevdocumentviewdelegate.cpp
    kdevdocumentview_part.cpp
    kdevdocumentmodel.cpp
    kdevdocumentselection.cpp
)


kde4_add_plugin(kdevdocumentview ${kdevdocumentview_PART_SRCS})
target_link_libraries(kdevdocumentview ${KDE4_KDECORE_LIBS} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDE4_KHTML_LIBS})

install(TARGETS kdevdocumentview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevdocumentview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevdocumentview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevdocumentview )

