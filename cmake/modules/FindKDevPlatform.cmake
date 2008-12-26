#
# Find the KDevelop Platform modules and sets various variables accordingly
#
# Example usage of this module:
# find_package(KDevPlatform 1.0.0 REQUIRED)
# 
# The version number and REQUIRED flag are optional. You can set CMAKE_PREFIX_PATH
# variable to help it find the required files and directories

# KDEVPLATFORM_FOUND                   - set to TRUE if the platform was found and the version is compatible FALSE otherwise
#
# KDEVPLATFORM_VERSION                 - The version number of kdevplatform
# KDEVPLATFORM_VERSION_MAJOR           - The major version number of kdevplatform
# KDEVPLATFORM_VERSION_MINOR           - The minor version number of kdevplatform
# KDEVPLATFORM_VERSION_PATCH           - The patch version number of kdevplatform
# KDEVPLATFORM_INCLUDE_DIR             - include dir of the platform, for example /usr/include/kdevplatform
# KDEVPLATFORM_INTERFACES_LIBRARY      - interfaces module library
# KDEVPLATFORM_LANGUAGE_LIBRARY        - language module library
# KDEVPLATFORM_OUTPUTVIEW_LIBRARY      - outputview module library
# KDEVPLATFORM_PROJECT_LIBRARY         - project module library
# KDEVPLATFORM_SUBLIME_LIBRARY         - sublime module library
# KDEVPLATFORM_SHELL_LIBRARY           - shell module library
# KDEVPLATFORM_TESTSHELL_LIBRARY       - module library to write tests that need to replace parts of shell
# KDEVPLATFORM_UTIL_LIBRARY            - util module library
# KDEVPLATFORM_VCS_LIBRARY             - vcs module library
# KDEVPLATFORM_VERITAS_LIBRARY         - test module library
# KDEVPLATFORM_SOURCEFORMATTER_LIBRARY - source formatter library
#
# Copyright 2007 Andreas Pakulat <apaku@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.

set( _args )
if( KDEVPLATFORM_FIND_VERSION_MAJOR )
    set( _args ${KDEVPLATFORM_FIND_VERSION_MAJOR} )
    if( KDEVPLATFORM_FIND_VERSION_MINOR )
        set( _args ${_args}.${KDEVPLATFORM_FIND_VERSION_MINOR} )
        if( KDEVPLATFORM_FIND_VERSION_PATCH )
            set( _args ${_args}.${KDEVPLATFORM_FIND_VERSION_PATCH} )
        endif( KDEVPLATFORM_FIND_VERSION_PATCH )
    endif( KDEVPLATFORM_FIND_VERSION_MINOR )
endif( KDEVPLATFORM_FIND_VERSION_MAJOR )

find_package( KDevPlatform ${_args} NO_MODULE )

if( NOT KDEVPLATFORM_FOUND AND KDEVPLATFORM_FIND_REQUIRED )
    message( FATAL_ERROR "Could not find KDevPlatform libraries, searched in '${CMAKE_PREFIX_PATH}' and '$ENV{CMAKE_PREFIX_PATH}'. You can set CMAKE_PREFIX_PATH to search in other directories." )
elseif( KDEVPLATFORM_FOUND AND NOT KDEVPLATFORM_FIND_QUIETLY )
    message( STATUS "Found KDevPlatform ${KDEVPLATFORM_VERSION}, using include dir ${KDEVPLATFORM_INCLUDE_DIR}." )
endif( NOT KDEVPLATFORM_FOUND AND KDEVPLATFORM_FIND_REQUIRED )


