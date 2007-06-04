#
# Find the KDevelop Platform modules and sets various variables accordingly
#
# KDEVPLATFORM_INCLUDE_DIR and KDEVPLATFORM_LIBRARY_DIR can be set to
# adjust the directories where the platform headers and libraries are searched
#
# KDEVPLATFORM_FOUND - set to TRUE if the platform was found (that is all its modules where found)
#                      FALSE otherwise
#
# KDEVPLATFORM_INCLUDE_DIR         - include dir of the platform, for example /usr/include/kdevplatform
#                                    can be set by the user to select different include dirs
# KDEVPLATFORM_INTERFACES_LIBRARY  - interfaces module library
# KDEVPLATFORM_EDITOR_LIBRARY      - editor module library
# KDEVPLATFORM_LANGUAGE_LIBRARY    - language module library
# KDEVPLATFORM_OUTPUTVIEW_LIBRARY  - outputview module library
# KDEVPLATFORM_PROJECT_LIBRARY     - project module library
# KDEVPLATFORM_SUBLIME_LIBRARY     - sublime module library
# KDEVPLATFORM_UTIL_LIBRARY        - util module library
# KDEVPLATFORM_VCS_LIBRARY         - vcs module library
#
# KDEVPLATFORM_LIBRARIES           - all platform module libraries
#
# Copyright (c) 2007 Andreas Pakulat <apaku@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if( KDEVPLATFORM_INCLUDE_DIR AND KDEVPLATFORM_LIBRARIES )
    set(KDEVPLATFORM_FIND_QUIETLY TRUE)
endif( KDEVPLATFORM_INCLUDE_DIR AND KDEVPLATFORM_LIBRARIES )

if( NOT KDEVPLATFORM_INCLUDE_DIR )
    find_path( KDEVPLATFORM_INCLUDE_DIR interfaces/iplugin.h
        PATHS
        /usr/include/kdevplatform
        /usr/local/include/kdevplatform
        DOC "kdevplatform include directory containing the various platform modules"
    )
else( NOT KDEVPLATFORM_INCLUDE_DIR )
    set(KDEVPLATFORM_INCLUDE_DIR ${KDEVPLATFORM_INCLUDE_DIR} CACHE PATH "kdevplatform include directory containing the various platform modules")
endif( NOT KDEVPLATFORM_INCLUDE_DIR )

if( NOT KDEVPLATFORM_LIBRARY_DIR )
    find_library( _platforminterfaces_lib kdevplatforminterfaces)
    get_filename_component(KDEVPLATFORM_LIBRARY_DIR _platforminterfaces_lib PATH )
endif( NOT KDEVPLATFORM_LIBRARY_DIR )
set( KDEVPLATFORM_LIBRARY_DIR ${KDEVPLATFORM_LIBRARY_DIR} CACHE PATH "path for kdevplatform libraries" )

message(STATUS "Using kdevplatform include dir: ${KDEVPLATFORM_INCLUDE_DIR}")
message(STATUS "Using kdevplatform library dir: ${KDEVPLATFORM_LIBRARY_DIR}")

find_library(KDEVPLATFORM_INTERFACES_LIBRARY libkdevplatforminterfaces
    PATHS
    ${KDEVPLATFORM_LIBRARY_DIR}
)

find_library(KDEVPLATFORM_EDITOR_LIBRARY kdevplatformeditor
    PATHS
    ${KDEVPLATFORM_LIBRARY_DIR}
)

find_library(KDEVPLATFORM_LANGUAGE_LIBRARY kdevplatformlanguage
    PATHS
    ${KDEVPLATFORM_LIBRARY_DIR}
)

find_library(KDEVPLATFORM_OUTPUTVIEW_LIBRARY kdevplatformoutputview
    PATHS
    ${KDEVPLATFORM_LIBRARY_DIR}
)

find_library(KDEVPLATFORM_PROJECT_LIBRARY kdevplatformproject
    PATHS
    ${KDEVPLATFORM_LIBRARY_DIR}
)

find_library(KDEVPLATFORM_SUBLIME_LIBRARY sublime
    PATHS
    ${KDEVPLATFORM_LIBRARY_DIR}
)

find_library(KDEVPLATFORM_UTIL_LIBRARY kdevplatformutil
    PATHS
    ${KDEVPLATFORM_LIBRARY_DIR}
)

find_library(KDEVPLATFORM_VCS_LIBRARY kdevplatformvcs
    PATHS
    ${KDEVPLATFORM_LIBRARY_DIR}
)

if( KDEVPLATFORM_INCLUDE_DIR
 AND KDEVPLATFORM_INTERFACES_LIBRARY
 AND KDEVPLATFORM_EDITOR_LIBRARY
 AND KDEVPLATFORM_LANGUAGE_LIBRARY
 AND KDEVPLATFORM_OUTPUTVIEW_LIBRARY
 AND KDEVPLATFORM_PROJECT_LIBRARY
 AND KDEVPLATFORM_SUBLIME_LIBRARY
 AND KDEVPLATFORM_UTIL_LIBRARY
 AND KDEVPLATFORM_VCS_LIBRARY )

    if( NOT KDEVPLATFORM_FIND_QUIETLY )
        message(STATUS "Found all KDevPlatform modules")
    endif( NOT KDEVPLATFORM_FIND_QUIETLY )

# if all modules found
    set(KDEVPLATFORM_FOUND TRUE)
    set(KDEVPLATFORM_LIBRARIES
        ${KDEVPLATFORM_INTERFACES_LIBRARY}
        ${KDEVPLATFORM_EDITOR_LIBRARY}
        ${KDEVPLATFORM_LANGUAGE_LIBRARY}
        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
        ${KDEVPLATFORM_PROJECT_LIBRARY}
        ${KDEVPLATFORM_SUBLIME_LIBRARY}
        ${KDEVPLATFORM_UTIL_LIBRARY}
    )

    set(KDEVPLATFORM_LIBRARIES ${KDEVPLATFORM_LIBRARIES} CACHE STRING "List of all kdevplatform module libraries")

else( KDEVPLATFORM_INCLUDE_DIR
 AND KDEVPLATFORM_INTERFACES_LIBRARY
 AND KDEVPLATFORM_EDITOR_LIBRARY
 AND KDEVPLATFORM_LANGUAGE_LIBRARY
 AND KDEVPLATFORM_OUTPUTVIEW_LIBRARY
 AND KDEVPLATFORM_PROJECT_LIBRARY
 AND KDEVPLATFORM_SUBLIME_LIBRARY
 AND KDEVPLATFORM_UTIL_LIBRARY
 AND KDEVPLATFORM_VCS_LIBRARY )

    if( KDevPlatform_FIND_REQUIRED)
        message(FATAL_ERROR "Couldn't find all platform modules")
    else
        message(STATUS "Couldn't find all platform modules")
    endif( KDevPlatform_FIND_REQUIRED)
# not all platform modules found
    set(KDEVPLATFORM_FOUND FALSE)

endif(KDEVPLATFORM_INCLUDE_DIR
 AND KDEVPLATFORM_INTERFACES_LIBRARY
 AND KDEVPLATFORM_EDITOR_LIBRARY
 AND KDEVPLATFORM_LANGUAGE_LIBRARY
 AND KDEVPLATFORM_OUTPUTVIEW_LIBRARY
 AND KDEVPLATFORM_PROJECT_LIBRARY
 AND KDEVPLATFORM_SUBLIME_LIBRARY
 AND KDEVPLATFORM_UTIL_LIBRARY
 AND KDEVPLATFORM_VCS_LIBRARY )

MARK_AS_ADVANCED(
    KDEVPLATFORM_INTERFACES_LIBRARY
    KDEVPLATFORM_EDITOR_LIBRARY
    KDEVPLATFORM_LANGUAGE_LIBRARY
    KDEVPLATFORM_OUTPUTVIEW_LIBRARY
    KDEVPLATFORM_PROJECT_LIBRARY
    KDEVPLATFORM_SUBLIME_LIBRARY
    KDEVPLATFORM_UTIL_LIBRARY
    KDEVPLATFORM_VCS_LIBRARY
)