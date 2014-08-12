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

option(BUILD_uiblocktester "Build a debug thread into the plugin that checks for UI-thread lockups" OFF)
if(BUILD_uiblocktester)
  add_definitions(-DDEBUG_UI_LOCKUP)
endif(BUILD_uiblocktester)

add_library(kdevcpplanguagesupport MODULE ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport
    kdevcpprpp
    kdevcppduchain
    kdevcppparser
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
