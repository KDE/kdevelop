cmake_minimum_required(VERSION 3.10)

project(%{APPNAME})

find_package(ECM "5.68.0" REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(KDevPlatform 5.2.40 REQUIRED)

add_subdirectory(src)

ecm_qt_install_logging_categories(
    EXPORT %{APPNAMEUC}
    FILE %{APPNAMELC}.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
