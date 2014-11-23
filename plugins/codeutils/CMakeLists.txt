project(codeutils)

########### install target ###############

set(kdevcodeutils_PART_SRCS
    codeutilsplugin.cpp
)

include_directories(CMAKE_CURRENT_BINARY_DIR)

add_library(kdevcodeutils MODULE ${kdevcodeutils_PART_SRCS})

target_link_libraries(kdevcodeutils
    KF5::Parts
    KF5::TextEditor
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::Language
)

add_subdirectory(doc_templates)

install(TARGETS kdevcodeutils DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )


########### install files ###############

configure_file(kdevcodeutils.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcodeutils.desktop)
kcoreaddons_desktop_to_json(kdevcodeutils ${CMAKE_CURRENT_BINARY_DIR}/kdevcodeutils.desktop)
install( FILES kdevcodeutils.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcodeutils )
