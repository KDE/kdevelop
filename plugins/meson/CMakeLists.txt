add_definitions(-DTRANSLATION_DOMAIN=\"kdevmesonmanager\")

ecm_qt_declare_logging_category(mesoncommon_LOG_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_Meson
    CATEGORY_NAME "kdevelop.plugins.meson"
)

kdevplatform_add_plugin(kdevmesonmanager
                        JSON kdevmesonmanager.json
                        SOURCES mesonmanager.cpp mesonbuilder.cpp mesonconfig.cpp mesonimportjob.cpp)

target_link_libraries(kdevmesonmanager
    Qt5::Concurrent
    KDev::Interfaces
    KDev::Project
    KDev::Util
    KDev::OutputView
)
