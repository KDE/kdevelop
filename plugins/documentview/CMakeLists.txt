project(documentview)
#add_subdirectory(settings)

########### next target ###############

set(kdevdocumentview_PART_SRCS
    kdevdocumentview.cpp
    kdevdocumentviewdelegate.cpp
    kdevdocumentviewplugin.cpp
    kdevdocumentmodel.cpp
    kdevdocumentselection.cpp
)


add_library(kdevdocumentview MODULE ${kdevdocumentview_PART_SRCS})
# FIXME: needs to be added for utilexport.h (needed because iproject MUST include util/path.h or moc breaks)
target_include_directories(kdevdocumentview PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../util)
target_link_libraries(kdevdocumentview ${KDEVPLATFORM_INTERFACES_LIBRARIES} KF5::TextEditor)


install(TARGETS kdevdocumentview DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevdocumentview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevdocumentview.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevdocumentview.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevdocumentview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevdocumentview )

