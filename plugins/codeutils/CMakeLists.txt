add_definitions(-DTRANSLATION_DOMAIN=\"kdevcodeutils\")
project(codeutils)

########### install target ###############

set(kdevcodeutils_PART_SRCS
    codeutilsplugin.cpp
)

include_directories(CMAKE_CURRENT_BINARY_DIR)

kdevplatform_add_plugin(kdevcodeutils JSON kdevcodeutils.json SOURCES ${kdevcodeutils_PART_SRCS})

target_link_libraries(kdevcodeutils
    KF5::Parts
    KF5::TextEditor
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::Language
)

add_subdirectory(doc_templates)

########### install files ###############

install( FILES kdevcodeutils.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcodeutils )
