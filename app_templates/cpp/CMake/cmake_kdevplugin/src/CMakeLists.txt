kdevplatform_add_plugin(%{APPNAMELC}
    JSON %{APPNAMELC}.json
    SOURCES %{APPNAMELC}.cpp
)

target_link_libraries(%{APPNAMELC}
    KDev::Interfaces
)
