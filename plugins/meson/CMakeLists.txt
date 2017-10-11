add_definitions(-DTRANSLATION_DOMAIN=\"kdevmesonmanager\")

set(mesonbuilder_SRCS
    mesonmanager.cpp
    mesonbuilder.cpp
    mesonconfig.cpp
    mesonimportjob.cpp
)

ecm_qt_declare_logging_category(mesonbuilder_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_Meson
    CATEGORY_NAME "kdevelop.plugins.meson"
)

kdevplatform_add_plugin(kdevmesonmanager
                        JSON kdevmesonmanager.json
                        SOURCES ${mesonbuilder_SRCS})

target_link_libraries(kdevmesonmanager
    Qt5::Concurrent
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::OutputView
)
