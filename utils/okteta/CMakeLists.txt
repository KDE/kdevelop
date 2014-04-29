project( okteta )


include_directories( ${okteta_SOURCE_DIR} ${okteta_BINARY_DIR} )

add_definitions(
    -DKASTEN_VERSION=${LIBKASTEN_VERSION}
)

include_directories(
  ${LIBKASTEN_INCLUDE_DIRS}
  ${LIBOKTETA_INCLUDE_DIRS}
  ${LIBOKTETAKASTEN_INCLUDE_DIRS}
)
SET( kdevokteta_PART_SRCS
    kastentoolviewwidget.cpp
    oktetatoolviewfactory.cpp
    oktetaplugin.cpp
    oktetadocument.cpp
    oktetaview.cpp
    oktetawidget.cpp
)

add_library(kdevokteta MODULE ${kdevokteta_PART_SRCS} )

TARGET_LINK_LIBRARIES( kdevokteta
    ${LIBOKTETAKASTEN_LIBRARIES}
    ${LIBOKTETA_LIBRARIES}
    ${LIBKASTEN_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_SUBLIME_LIBRARIES}
    ${KDEVPLATFORM_SHELL_LIBRARIES}
    KF5::KDELibs4Support
    KF5::WidgetsAddons
)

install(TARGETS kdevokteta DESTINATION ${PLUGIN_INSTALL_DIR})


configure_file(kdevokteta.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevokteta.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevokteta.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES kdevokteta.rc DESTINATION  ${DATA_INSTALL_DIR}/kdevokteta )
