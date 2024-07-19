qt_add_executable(%{APPNAMELC}
    main.cpp
    %{APPNAMELC}.ui
    %{APPNAMELC}.cpp
)

# Use the Widgets module from Qt 6.
target_link_libraries(%{APPNAMELC} PRIVATE Qt6::Widgets)

# Install the executable
install(TARGETS %{APPNAMELC} DESTINATION bin)
