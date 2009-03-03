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
# The following macros are added (from KDevPlatformMacros.cmake):
#
#  KDEVPLATFORM_ADD_APP_TEMPLATES( template1 ... templateN )
#    Use this to get packaged template archives for the given templates.
#    Parameters should be the directories containing the templates.
#
# Copyright 2007 Andreas Pakulat <apaku@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.

set(_KDevPlatform_FIND_QUIETLY ${KDevPlatform_FIND_QUIETLY})
find_package( KDevPlatform ${KDevPlatform_FIND_VERSION} NO_MODULE )
set(KDevPlatform_FIND_QUIETLY ${_KDevPlatform_FIND_QUIETLY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KDevPlatform DEFAULT_MSG KDevPlatform_CONFIG )

