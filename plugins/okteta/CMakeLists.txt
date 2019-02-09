add_definitions(-DTRANSLATION_DOMAIN=\"kdevokteta\")

SET( kdevokteta_PART_SRCS
    kastentoolviewwidget.cpp
    oktetatoolviewfactory.cpp
    oktetaplugin.cpp
    oktetadocument.cpp
    oktetaview.cpp
    oktetawidget.cpp
)

if (NOT KastenCore_VERSION VERSION_LESS 0.4.0)
    set(USE_KASTEN_CONTROLLER_FACTORIES TRUE)
endif()
configure_file(config-kdevokteta.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdevokteta.h)

qt5_add_resources(kdevokteta_PART_SRCS kdevokteta.qrc)
kdevplatform_add_plugin(kdevokteta JSON kdevokteta.json SOURCES
	${kdevokteta_PART_SRCS})

TARGET_LINK_LIBRARIES( kdevokteta
    KDev::Interfaces
    KDev::Project
    KDev::Sublime
    KDev::Shell
    KF5::WidgetsAddons
    OktetaKastenControllers
    KastenControllers
)
