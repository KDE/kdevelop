add_definitions(-DTRANSLATION_DOMAIN=\"kdevshellcheck\")

##find_program() ??
#find_package(???? QUIET)
#set_package_properties(shellcheck PROPERTIES
#    DESCRIPTION "A static analysis and linting tool for sh/bash scripts"
#    URL "https://www.shellcheck.net/"
#    PURPOSE "Recommended: required by the non-essential shellcheck plugin"
#    TYPE RUNTIME
#)

set(kdevshellcheck_SRCS
    shellcheck.cpp
)

ecm_qt_declare_logging_category(kdevshellcheck_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_SHELLCHECK
    CATEGORY_NAME "kdevelop.plugins.shellcheck"
)

kdevplatform_add_plugin(kdevshellcheck
    JSON kdevshellcheck.json
    SOURCES ${kdevshellcheck_SRCS}
)

target_link_libraries(kdevshellcheck
    KDev::Interfaces
)
