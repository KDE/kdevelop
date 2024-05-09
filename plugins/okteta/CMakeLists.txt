add_definitions(-DTRANSLATION_DOMAIN=\"kdevokteta\")

SET( kdevokteta_PART_SRCS
    kastentoolviewwidget.cpp
    oktetatoolviewfactory.cpp
    oktetaplugin.cpp
    oktetadocument.cpp
    oktetaview.cpp
    oktetawidget.cpp
)

qt_add_resources(kdevokteta_PART_SRCS kdevokteta.qrc)
kdevplatform_add_plugin(kdevokteta SOURCES
	${kdevokteta_PART_SRCS})

TARGET_LINK_LIBRARIES( kdevokteta
    KDev::Interfaces
    KDev::Project
    KDev::Sublime
    KDev::Shell
    KF6::WidgetsAddons
    OktetaKastenControllers
    KastenControllers
)
