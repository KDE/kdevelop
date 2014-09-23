add_subdirectory(dataengine)
add_subdirectory(applet)

find_package(KF5Runner)
set_package_properties(KF5Runner PROPERTIES
    PURPOSE "Framework for Plasma runners. For enabling the KDevelop runner"
    URL "http://www.kde.org"
    TYPE OPTIONAL)
if(KF5Runner_FOUND)
    add_subdirectory(runner)
endif()
