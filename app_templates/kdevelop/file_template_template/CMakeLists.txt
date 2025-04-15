project(%{APPNAMELC})

cmake_minimum_required(VERSION 3.10)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)
include(KDEPackageAppTemplates)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(DEFINED ENV{XDG_DATA_HOME})
        set(KDE_INSTALL_DATADIR "$ENV{XDG_DATA_HOME}" CACHE PATH "default install prefix" FORCE)
    else()
        set(KDE_INSTALL_DATADIR "$ENV{HOME}/.local/share" CACHE PATH "default install prefix" FORCE)
    endif()
endif()

kde_package_app_templates(TEMPLATES "%{APPNAMELC}" INSTALL_DIR "${KDE_INSTALL_DATADIR}/kdevfiletemplates/templates")
