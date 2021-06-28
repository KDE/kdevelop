set(%{APPNAMELC}_SRCS
    %{APPNAMELC}.cpp
)

ecm_qt_declare_logging_category(%{APPNAMELC}_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_%{APPNAMEUC}
    CATEGORY_NAME "kdevelop.plugins.%{APPNAMELC}"
    DESCRIPTION "KDevelop plugin: %{APPNAME}"
    EXPORT %{APPNAMEUC}
)

kdevplatform_add_plugin(%{APPNAMELC}
    SOURCES ${%{APPNAMELC}_SRCS}
)

target_link_libraries(%{APPNAMELC}
    KDev::Interfaces
)
