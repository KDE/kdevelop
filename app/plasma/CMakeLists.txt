add_subdirectory(kdevelopsessionswatch)

add_subdirectory(applet)

find_package(KF6Runner)
set_package_properties(KF6Runner PROPERTIES
    PURPOSE "Framework for Plasma runners. For enabling the KDevelop runner"
    URL "https://www.kde.org"
    TYPE OPTIONAL
)
if(KF6Runner_FOUND)
    add_subdirectory(runner)
endif()
