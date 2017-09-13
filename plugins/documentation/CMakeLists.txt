find_package(Qt5Help CONFIG)

set_package_properties(Qt5Help PROPERTIES
    PURPOSE "The Help module for the Qt toolkit, needed for the qthelp plugin"
    URL "https://www.qt.io/"
    TYPE OPTIONAL)
if(Qt5Help_FOUND)
    ecm_optional_add_subdirectory(qthelp)
endif()

ecm_optional_add_subdirectory(manpage)
