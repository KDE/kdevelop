project(%{APPNAMELC})

cmake_minimum_required(VERSION 3.0)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)
include(KDEPackageAppTemplates)

kde_package_app_templates(TEMPLATES "%{APPNAMELC}" INSTALL_DIR "${KDE_INSTALL_DATADIR}/kdevfiletemplates/templates")
