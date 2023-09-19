add_subdirectory(kdevelopsessionswatch)

add_subdirectory(applet)

find_package(KF${KF_MAJOR_VERSION}Runner CONFIG)
set_package_properties(KF${KF_MAJOR_VERSION}Runner PROPERTIES
    PURPOSE "Framework for Plasma runners. For enabling the KDevelop runner"
    URL "https://www.kde.org"
    TYPE OPTIONAL)
if(KF${KF_MAJOR_VERSION}Runner_FOUND)
    add_subdirectory(runner)
endif()
