#
# Find the KDevelop Platform modules and sets various variables accordingly
#
# Example usage of this module:
# find_package(KDevPlatform 1.0.0 REQUIRED)
# 
# The version number and REQUIRED flag are optional. You can set CMAKE_PREFIX_PATH
# variable to help it find the required files and directories

# KDevPlatform_FOUND                   - set to TRUE if the platform was found and the version is compatible FALSE otherwise
#
# KDevPlatform_VERSION                 - The version number of kdevplatform
# KDevPlatform_VERSION_MAJOR           - The major version number of kdevplatform
# KDevPlatform_VERSION_MINOR           - The minor version number of kdevplatform
# KDevPlatform_VERSION_PATCH           - The patch version number of kdevplatform
# KDevPlatform_INCLUDE_DIR             - include dir of the platform, for example /usr/include/kdevplatform
# KDevPlatform_INTERFACES_LIBRARY      - interfaces module library
# KDevPlatform_LANGUAGE_LIBRARY        - language module library
# KDevPlatform_OUTPUTVIEW_LIBRARY      - outputview module library
# KDevPlatform_PROJECT_LIBRARY         - project module library
# KDevPlatform_SUBLIME_LIBRARY         - sublime module library
# KDevPlatform_SHELL_LIBRARY           - shell module library
# KDevPlatform_UTIL_LIBRARY            - util module library
# KDevPlatform_VCS_LIBRARY             - vcs module library
# KDevPlatform_VERITAS_LIBRARY         - test module library
# KDevPlatform_SOURCEFORMATTER_LIBRARY - source formatter library
#
# Copyright 2007 Andreas Pakulat <apaku@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.

set( _args )
if( KDevPlatform_FIND_VERSION_MAJOR )
    set( _args ${KDevPlatform_FIND_VERSION_MAJOR} )
    if( KDevPlatform_FIND_VERSION_MINOR )
        set( _args ${_args}.${KDevPlatform_FIND_VERSION_MINOR} )
        if( KDevPlatform_FIND_VERSION_PATCH )
            set( _args ${_args}.${KDevPlatform_FIND_VERSION_PATCH} )
        endif( KDevPlatform_FIND_VERSION_PATCH )
    endif( KDevPlatform_FIND_VERSION_MINOR )
endif( KDevPlatform_FIND_VERSION_MAJOR )

find_package( KDevPlatform ${_args} NO_MODULE )

if( NOT KDevPlatform_FOUND AND KDevPlatform_FIND_REQUIRED )
    message( FATAL "Could not find KDevPlatform libraries, searched in ${CMAKE_PREFIX_PATH} and $ENV{CMAKE_PREFIX_PATH}. You can set CMAKE_PREFIX_PATH to search in other directories." )
elseif( KDevPlatform_FOUND AND NOT KDevPlatform_FIND_QUIETLY )
    message( STATUS "Found KDevPlatform ${KDevPlatform_VERSION}, using include dir ${KDevPlatform_INCLUDE_DIR}." )
endif( NOT KDevPlatform_FOUND AND KDevPlatform_FIND_REQUIRED )


