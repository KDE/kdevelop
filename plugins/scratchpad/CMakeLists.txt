add_definitions(-DTRANSLATION_DOMAIN=\"kdevscratchpad\")
set(scratchpad_SRCS
    scratchpad.cpp
    scratchpadview.cpp
    scratchpadjob.cpp
)

ki18n_wrap_ui(scratchpad_SRCS scratchpadview.ui)

qt_add_resources(scratchpad_SRCS kdevscratchpad.qrc)

declare_qt_logging_category(scratchpad_SRCS
    TYPE PLUGIN
    IDENTIFIER PLUGIN_SCRATCHPAD
    CATEGORY_BASENAME "scratchpad"
)

kdevplatform_add_plugin(scratchpad
    SOURCES ${scratchpad_SRCS}
)

target_link_libraries(scratchpad
    KDev::Interfaces
    KDev::Sublime
    KDev::Util
    KDev::OutputView
)
