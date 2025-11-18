#
# KDevelop Platform Private Macros
#
# The following macros are defined here:
#
#  KDEVPLATFORM_ADD_LIBRARY( <targetname> SOURCES <src1> [<src2> [...]] )
#    Use this to add libraries part of the KDevPlatform
#
# SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
#
# SPDX-License-Identifier: BSD-3-Clause

function(kdevplatform_add_library target)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(KDEV_ADD_LIBRARY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    string(REPLACE "KDevPlatform" "" shortTargetName ${target})
    if (${shortTargetName} STREQUAL ${target})
        message(FATAL_ERROR "Target passed to kdevplatform_add_library needs to start with \"KDevPlatform\", was \"${target}\"")
    endif()

    string(TOLOWER ${shortTargetName} shortTargetNameToLower)

    add_library(${target} ${KDEV_ADD_LIBRARY_SOURCES})
    add_library(KDev::${shortTargetName} ALIAS ${target})

    generate_export_header(${target} EXPORT_FILE_NAME ${shortTargetNameToLower}export.h)

    target_include_directories(${target}
            INTERFACE   "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kdevplatform>"
                        "$<BUILD_INTERFACE:${KDevPlatform_SOURCE_DIR}>" "$<BUILD_INTERFACE:${KDevPlatform_BINARY_DIR}>"
    )
    set_target_properties(${target} PROPERTIES
        VERSION ${KDEVPLATFORM_VERSION}
        SOVERSION ${KDEVPLATFORM_SOVERSION}
        EXPORT_NAME ${shortTargetName}
    )

    install(TARGETS ${target} EXPORT KDevPlatformTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${shortTargetNameToLower}export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/${shortTargetNameToLower} COMPONENT Devel)
endfunction()
