add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecuteplasmoid\")

set(kdevexecuteplasmoid_PART_SRCS
    executeplasmoidplugin.cpp
    plasmoidexecutionconfig.cpp
    plasmoidexecutionjob.cpp
)
ecm_qt_declare_logging_category(kdevexecuteplasmoid_PART_SRCS
    HEADER debug.h
    IDENTIFIER EXECUTEPLASMOID
    CATEGORY_NAME "kdevelop.launchers.executeplasmoid"
)
ki18n_wrap_ui(kdevexecuteplasmoid_PART_SRCS
    plasmoidexecutionconfig.ui
)
kdevplatform_add_plugin(kdevexecuteplasmoid JSON kdevexecuteplasmoid.json SOURCES ${kdevexecuteplasmoid_PART_SRCS})
target_link_libraries(kdevexecuteplasmoid
    KDev::Language
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView)
