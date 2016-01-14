find_package( OktetaKastenControllers )

set_package_properties( OktetaKastenControllers PROPERTIES
    PURPOSE "Required for building Okteta KDevelop plugin."
    URL "http://kde.org/"
    TYPE OPTIONAL)

if( OktetaKastenControllers_FOUND )
    find_package( KastenControllers )

    set_package_properties(KastenControllers PROPERTIES
        PURPOSE "Required for building Okteta KDevelop plugin."
        URL "http://kde.org/"
        TYPE OPTIONAL)
endif()

if( OktetaKastenControllers_FOUND AND KastenControllers_FOUND)
    add_subdirectory(okteta)
endif()
