add_definitions(-DTRANSLATION_DOMAIN=\"kdevcmake\")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/parser)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(icons)

declare_qt_logging_category(cmake_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER CMAKE
    CATEGORY_BASENAME "cmake"
)

declare_qt_logging_category(cmake_testing_LOG_SRCS
    TYPE PLUGIN
    HEADER "debug_testing.h"
    IDENTIFIER CMAKE_TESTING
    CATEGORY_BASENAME "cmake.testing"
    DESCRIPTION "cmake testing"
)

set( cmakecommon_SRCS
  parser/cmListFileLexer.c
  parser/cmakecachereader.cpp
  parser/cmakelistsparser.cpp
  parser/cmakeduchaintypes.cpp
  cmakeutils.cpp
  cmakeextraargumentshistory.cpp
  cmakebuilddirchooser.cpp
  cmakeserver.cpp
  cmakefileapi.cpp
  cmakeprojectdata.cpp
  ${cmake_LOG_SRCS}
)
set_source_files_properties(parser/cmListFileLexer.c PROPERTIES COMPILE_FLAGS "-DYY_NO_INPUT -DYY_NO_UNPUT")

set( cmakecommon_UI
  cmakebuilddirchooser.ui
)

set( cmakemanager_SRCS
  testing/ctestutils.cpp
  testing/ctestfindjob.cpp
  testing/ctestrunjob.cpp
  testing/ctestsuite.cpp
  testing/qttestdelegate.cpp
  cmakeimportjsonjob.cpp
  cmakeserverimportjob.cpp
  cmakefileapiimportjob.cpp
  cmakenavigationwidget.cpp
  cmakemanager.cpp
  cmakemodelitems.cpp
  duchain/cmakeparsejob.cpp
  duchain/usebuilder.cpp
  duchain/declarationbuilder.cpp
  duchain/contextbuilder.cpp
  cmakecodecompletionmodel.cpp
  ${cmake_LOG_SRCS}
  ${cmake_testing_LOG_SRCS}
)

set( cmakesettings_SRCS
  settings/cmakepreferences.cpp
  settings/cmakecachemodel.cpp
  settings/cmakecachedelegate.cpp
  settings/cmakecachemodel.cpp
)
ki18n_wrap_ui(cmakesettings_SRCS settings/cmakebuildsettings.ui)

set( cmakedoc_SRCS
  cmakedocumentation.cpp
  cmakehelpdocumentation.cpp
  cmakecommandscontents.cpp
)

if(MSVC)
    add_definitions(-DYY_NO_UNISTD_H)
endif()

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target

kconfig_add_kcfg_files( cmakecommon_SRCS cmakebuilderconfig.kcfgc )
ki18n_wrap_ui( cmakecommon_SRCS ${cmakecommon_UI} )
kdevelop_add_private_library(KDevCMakeCommon SOURCES ${cmakecommon_SRCS})
target_link_libraries(KDevCMakeCommon
    PUBLIC
        kdevmakefileresolver
        KDev::Interfaces
        KDev::Project
        KDev::Util
        KDev::Language
        KF6::TextEditor
        KF6::KIOWidgets
)

add_library( kdevcmakemanagernosettings STATIC ${cmakemanager_SRCS})
target_compile_definitions(kdevcmakemanagernosettings PRIVATE CMAKEMANAGER_NO_SETTINGS)
target_link_libraries(kdevcmakemanagernosettings
    KDevCMakeCommon
    KDev::Util
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Sublime
    KDev::OutputView
    KF6::KIOWidgets
    KF6::TextEditor
    Qt::Concurrent
)

kdevplatform_add_plugin(KDevCMakeManager SOURCES ${cmakemanager_SRCS} ${cmakesettings_SRCS})
target_link_libraries(KDevCMakeManager
    KDevCMakeCommon
    KDev::Util
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Shell
    KDev::OutputView
    KF6::KIOWidgets
    KF6::TextEditor
    Qt::Concurrent
)

kdevplatform_add_plugin(KDevCMakeDocumentation SOURCES ${cmakedoc_SRCS})
target_link_libraries(KDevCMakeDocumentation
    KDevCMakeCommon
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Documentation
    KDev::Sublime
    KF6::ItemModels
    KF6::TextEditor
)
