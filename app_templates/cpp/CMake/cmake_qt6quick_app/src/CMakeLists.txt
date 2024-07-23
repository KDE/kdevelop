qt_add_executable(%{APPNAMELC}
    main.cpp
)

qt_add_qml_module(%{APPNAMELC}
    URI %{APPNAMEID}
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

# Use the QtQuick module from Qt 6.
target_link_libraries(%{APPNAMELC} PRIVATE Qt6::Quick)

# Install the executable
install(TARGETS %{APPNAMELC} DESTINATION bin)
