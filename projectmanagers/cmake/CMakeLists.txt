project(cmakemanager)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/parser)

add_subdirectory(tests)
add_subdirectory(icons)

# enable this if you want to have the cmake debug visitor run on each CMakeLists.txt
# the debug visitor prints out the Ast for the CMakeLists.txt file.
#add_definitions( -DCMAKEDEBUGVISITOR )

add_definitions( -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

set( cmakecommon_SRCS
  parser/cmListFileLexer.cpp
#   parser/astfactory.cpp
#   parser/cmakelistsparser.cpp
#   parser/cmakeast.cpp
#   parser/cmakecondition.cpp
#   parser/cmakeprojectvisitor.cpp
#   parser/variablemap.cpp
#   parser/cmakedebugvisitor.cpp
  parser/cmakecachereader.cpp
  parser/cmakelistsparser.cpp
  parser/cmakeduchaintypes.cpp
#   parser/cmakeparserutils.cpp
#   parser/cmakeduchaintypes.cpp
#   parser/generationexpressionsolver.cpp
  settings/cmakecachemodel.cpp
  cmakeutils.cpp
  cmakebuilddirchooser.cpp
#   cmakemodelitems.cpp
  debug.cpp

 )

set( cmakecommon_UI
  cmakebuilddirchooser.ui
)

set( cmakemanager_SRCS
#   testing/ctestutils.cpp
#   testing/ctestfindjob.cpp
#   testing/ctestrunjob.cpp
#   testing/ctestsuite.cpp
#   testing/qttestdelegate.cpp
  cmakeimportjsonjob.cpp
  cmakenavigationwidget.cpp
  cmakemanager.cpp
  duchain/cmakeparsejob.cpp
  duchain/usebuilder.cpp
  duchain/declarationbuilder.cpp
  duchain/contextbuilder.cpp
  cmakecodecompletionmodel.cpp
#   cmakecommitchangesjob.cpp
#   cmakeimportjob.cpp
#   cmakeedit.cpp
  debug.cpp
)

set( cmakemanager_UI
  cmakepossibleroots.ui
)

set( cmakesettings_SRCS
  settings/cmakepreferences.cpp
  settings/cmakecachemodel.cpp
  settings/cmakecachedelegate.cpp
  debug.cpp
)

set( cmakesettings_UI
  settings/cmakebuildsettings.ui
)

set( cmakedoc_SRCS
  cmakedocumentation.cpp
  cmakehelpdocumentation.cpp
)

remove_definitions( -DQT_NO_STL )

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target

qt5_wrap_ui( cmakecommon_SRCS ${cmakecommon_UI} )
add_library( kdevcmakecommon SHARED ${cmakecommon_SRCS} )
target_link_libraries( kdevcmakecommon
                        KF5::TextEditor KF5::KDELibs4Support KDev::Interfaces KDev::Project KDev::Util
                        ${QT_QTSCRIPT_LIBRARY} KDev::Language
                        )
generate_export_header(kdevcmakecommon EXPORT_FILE_NAME cmakecommonexport.h)

qt5_wrap_ui( cmakemanager_SRCS ${cmakemanager_UI} )
add_library(kdevcmakemanager MODULE ${cmakemanager_SRCS} ../custommake/makefileresolver/makefileresolver.cpp)
target_link_libraries( kdevcmakemanager  KF5::KIOWidgets KDev::Util
                        KDev::Interfaces kdevcmakecommon
                        KDev::Project KDev::Language
                        KDev::OutputView KF5::TextEditor KF5::KDELibs4Support Qt5::Concurrent)

add_library(kdevcmakedocumentation MODULE ${cmakedoc_SRCS} )
target_link_libraries( kdevcmakedocumentation
                        KDev::Interfaces kdevcmakecommon
                        KDev::Project KDev::Language
                        KDev::Documentation
                        KF5::TextEditor Qt5::WebKitWidgets)

qt5_wrap_ui( cmakesettings_SRCS ${cmakesettings_UI} )
add_library(kcm_kdevcmake_settings MODULE ${cmakesettings_SRCS} )
target_link_libraries( kcm_kdevcmake_settings KF5::KIOWidgets KF5::KDELibs4Support
                       KDev::Interfaces KDev::Project KDev::Util
                       kdevcmakecommon )

install(TARGETS kdevcmakemanager DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )
install(TARGETS kdevcmakedocumentation DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION} )
install(TARGETS kcm_kdevcmake_settings DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION}/kcm )
install(TARGETS kdevcmakecommon ${INSTALL_TARGETS_DEFAULT_ARGS} )
configure_file(kdevcmakemanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakemanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
configure_file(kdevcmakedocumentation.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakedocumentation.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakedocumentation.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
configure_file(kcm_kdevcmake_settings.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevcmake_settings.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kdevcmake_settings.desktop DESTINATION ${SERVICES_INSTALL_DIR})
