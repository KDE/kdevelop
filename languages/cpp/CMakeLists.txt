project(cpp)
include(CppBuildConfig.cmake)

include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp/parser
    ${KDevelop_SOURCE_DIR}/languages/cpp/cppduchain
    ${KDevelop_SOURCE_DIR}/languages/plugins
)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9007 )

add_subdirectory(parser)
add_subdirectory(cppduchain)
add_subdirectory(tests)
########### next target ###############

set(kdevcpplanguagesupport_PART_SRCS
    cpplanguagesupport.cpp
    includepathcomputer.cpp
    cppparsejob.cpp
    preprocessjob.cpp
    cpphighlighting.cpp
    cpputils.cpp
    makefileresolver.cpp
    setuphelpers.cpp
    quickopen.cpp
    
    codecompletion/model.cpp
    codecompletion/worker.cpp
    codecompletion/context.cpp
    codecompletion/item.cpp
    codecompletion/helpers.cpp
    codecompletion/missingincludeitem.cpp
    codecompletion/implementationhelperitem.cpp

#    codegen/cppnewclass.cpp
    codegen/simplerefactoring.cpp
    codegen/adaptsignatureassistant.cpp
    codegen/adaptsignatureaction.cpp
    codegen/unresolvedincludeassistant.cpp
#     codegen/cppclasshelper.cpp #TODO reenable when grantlee is ported
#     codegen/makeimplementationprivate.cpp
)

option(BUILD_kdev_includepathresolver "Build the includepath resolver debugging tool" OFF)
if(BUILD_kdev_includepathresolver)
  add_executable(kdev_includepathresolver includepathresolver.cpp)
  set_target_properties( kdev_includepathresolver PROPERTIES COMPILE_FLAGS -DTEST )
  target_link_libraries( kdev_includepathresolver
  ${KDEVPLATFORM_INTERFACES_LIBRARIES}  ${KDEVPLATFORM_PROJECT_LIBRARIES}
  Qt5::Core Qt5::Gui  ${KDEVPLATFORM_LANGUAGE_LIBRARIES} )
  install(TARGETS kdev_includepathresolver ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif(BUILD_kdev_includepathresolver)

option(BUILD_uiblocktester "Build a debug thread into the plugin that checks for UI-thread lockups" OFF)
if(BUILD_uiblocktester)
  add_definitions(-DDEBUG_UI_LOCKUP)
endif(BUILD_uiblocktester)

add_library(kdevcpplanguagesupport MODULE ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport
    kdev4cpprpp
    kdev4cppduchain
    kdev4cppparser
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDE4_THREADWEAVER_LIBRARIES}
    
    KF5::TextEditor
)

install(TARGETS kdevcpplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

configure_file(kdevcppsupport.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcppsupport.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcppsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevcppsupport.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcppsupport)
