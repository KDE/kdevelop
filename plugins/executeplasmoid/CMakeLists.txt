add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecuteplasmoid\")

set(kdevexecuteplasmoid_PART_SRCS
    executeplasmoidplugin.cpp
    plasmoidexecutionconfig.cpp
    plasmoidexecutionjob.cpp
)
declare_qt_logging_category(kdevexecuteplasmoid_PART_SRCS
    TYPE PLUGIN
    IDENTIFIER EXECUTEPLASMOID
    CATEGORY_BASENAME "executeplasmoid"
)
ki18n_wrap_ui(kdevexecuteplasmoid_PART_SRCS
    plasmoidexecutionconfig.ui
)
kdevplatform_add_plugin(kdevexecuteplasmoid SOURCES ${kdevexecuteplasmoid_PART_SRCS})
target_link_libraries(kdevexecuteplasmoid
    KDev::Language
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView
    KDev::IExecute
)
