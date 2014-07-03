project(cmakemanager)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/parser)

# add_subdirectory(tests)
add_subdirectory(icons)

# enable this if you want to have the cmake debug visitor run on each CMakeLists.txt
# the debug visitor prints out the Ast for the CMakeLists.txt file.
#add_definitions( -DCMAKEDEBUGVISITOR )

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9042 -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

set( cmakecommon_SRCS
#   parser/cmListFileLexer.cpp
#   parser/astfactory.cpp
#   parser/cmakelistsparser.cpp
#   parser/cmakeast.cpp
#   parser/cmakecondition.cpp
#   parser/cmakeprojectvisitor.cpp
#   parser/variablemap.cpp
#   parser/cmakedebugvisitor.cpp
  parser/cmakecachereader.cpp
#   parser/cmakeparserutils.cpp
#   parser/cmakeduchaintypes.cpp
#   parser/generationexpressionsolver.cpp
  settings/cmakecachemodel.cpp
  cmakeutils.cpp
  cmakebuilddirchooser.cpp
#   cmakemodelitems.cpp
 
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
  cmakenavigationwidget.cpp
  cmakemanager.cpp
#   cmakecodecompletionmodel.cpp
#   cmakecommitchangesjob.cpp
#   cmakeimportjob.cpp
#   cmakeedit.cpp
)

set( cmakemanager_UI
  cmakepossibleroots.ui
)

set( cmakesettings_SRCS
  settings/cmakepreferences.cpp
  settings/cmakecachemodel.cpp
  settings/cmakecachedelegate.cpp
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
add_library( kdev4cmakecommon SHARED ${cmakecommon_SRCS} )
target_link_libraries( kdev4cmakecommon LINK_PRIVATE ${KDE4_KDEUI_LIBS}
                        KF5::TextEditor ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES}
                        ${QT_QTSCRIPT_LIBRARY} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
                        )

qt5_wrap_ui( cmakemanager_SRCS ${cmakemanager_UI} )
add_library(kdevcmakemanager MODULE ${cmakemanager_SRCS} )
target_link_libraries( kdevcmakemanager ${KDE4_KDEUI_LIBS} KF5::KIOWidgets ${KDEVPLATFORM_UTIL_LIBRARIES}
                        ${KDEVPLATFORM_INTERFACES_LIBRARIES} kdev4cmakecommon
                        ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
                        ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES} KF5::TextEditor Qt5::Concurrent)
                        
add_library(kdevcmakedocumentation MODULE ${cmakedoc_SRCS} )
target_link_libraries( kdevcmakedocumentation
                        ${KDEVPLATFORM_INTERFACES_LIBRARIES} kdev4cmakecommon
                        ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
                        ${KDEVPLATFORM_DOCUMENTATION_LIBRARIES}
                        KF5::TextEditor Qt5::WebKitWidgets)

qt5_wrap_ui( cmakesettings_SRCS ${cmakesettings_UI} )
add_library(kcm_kdevcmake_settings MODULE ${cmakesettings_SRCS} )
target_link_libraries( kcm_kdevcmake_settings KF5::KIOWidgets
                       ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES}
                       kdev4cmakecommon )

install(TARGETS kdevcmakemanager DESTINATION ${PLUGIN_INSTALL_DIR} )
install(TARGETS kdevcmakedocumentation DESTINATION ${PLUGIN_INSTALL_DIR} )
install(TARGETS kcm_kdevcmake_settings DESTINATION ${PLUGIN_INSTALL_DIR} )
install(TARGETS kdev4cmakecommon ${INSTALL_TARGETS_DEFAULT_ARGS} )
configure_file(kdevcmakemanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakemanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakemanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
configure_file(kdevcmakedocumentation.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakedocumentation.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcmakedocumentation.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES kcm_kdevcmake_settings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
