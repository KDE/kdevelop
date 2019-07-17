cmake_minimum_required(VERSION 3.0)

project(%{APPNAME})

find_package(ECM "5.14.0" REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(KDevPlatform 5.2.40 REQUIRED)

add_subdirectory(src)

# kdebugsettings file
if (ECM_VERSION VERSION_GREATER "5.58.0")
    install(FILES %{APPNAMELC}.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES %{APPNAMELC}.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
