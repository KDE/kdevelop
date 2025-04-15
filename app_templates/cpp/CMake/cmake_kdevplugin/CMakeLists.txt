cmake_minimum_required(VERSION 3.16)

project(%{APPNAME})

set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    I18n
)

find_package(KDevPlatform 6.0 REQUIRED)

add_subdirectory(src)

ecm_qt_install_logging_categories(
    EXPORT %{APPNAMEUC}
    FILE %{APPNAMELC}.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
