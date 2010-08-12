project( okteta )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${okteta_SOURCE_DIR}/cmake/modules )

find_package(KDE4 4.5.0 REQUIRED)
find_package(KDevPlatform 1.0.0 REQUIRED)

include_directories( ${KDE4_INCLUDES} ${KDEVPLATFORM_INCLUDE_DIR} )
include_directories( ${okteta_SOURCE_DIR} ${okteta_BINARY_DIR} )

# add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9038 )

macro_optional_find_package( LibOktetaKasten )
macro_optional_find_package( LibOkteta )
macro_optional_find_package( LibKasten )

macro_log_feature( LIBOKTETAKASTEN_FOUND
  "libkasten" "Okteta Kasten libraries" "kdeutils/okteta/kasten" FALSE
  "" "Required for building Okteta KDevelop plugin."
)
macro_log_feature( LIBOKTETA_FOUND
  "libokteta" "Okteta libraries" "kdeutils/okteta" FALSE
  "" "Required for building Okteta KDevelop plugin."
)
macro_log_feature( LIBKASTEN_FOUND
  "libkasten" "Kasten libraries" "kdeutils/okteta/libs" FALSE
  "" "Required for building Okteta KDevelop plugin."
)
include_directories(
  ${LIBKASTEN_INCLUDE_DIR}
  ${LIBOKTETA_INCLUDE_DIR}
)
SET( kdevokteta_PART_SRCS
    kastentoolviewwidget.cpp
    oktetatoolviewfactory.cpp
    oktetaplugin.cpp
    oktetadocument.cpp
    oktetaview.cpp
    oktetawidget.cpp
)

kde4_add_plugin( kdevokteta ${kdevokteta_PART_SRCS} )

TARGET_LINK_LIBRARIES( kdevokteta
    ${LIBOKTETAKASTEN_LIBRARIES}
    ${LIBOKTETA_LIBRARIES}
    ${LIBKASTEN_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_SUBLIME_LIBRARIES}
    ${KDEVPLATFORM_SHELL_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
)

install(TARGETS kdevokteta DESTINATION ${PLUGIN_INSTALL_DIR})


install(FILES kdevokteta.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES kdevokteta.rc DESTINATION  ${DATA_INSTALL_DIR}/kdevokteta )
