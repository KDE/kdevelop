project(cpp)
add_definitions(-DTRANSLATION_DOMAIN=\"kdevcpp\")
include(CppBuildConfig.cmake)

include_directories(
    ${KDevelop_SOURCE_DIR}/languages/cpp/parser
    ${KDevelop_SOURCE_DIR}/languages/cpp/cppduchain
    ${KDevelop_SOURCE_DIR}/languages/plugins
)

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
    codegen/cppclasshelper.cpp
#     codegen/makeimplementationprivate.cpp
)

option(BUILD_uiblocktester "Build a debug thread into the plugin that checks for UI-thread lockups" OFF)
if(BUILD_uiblocktester)
  add_definitions(-DDEBUG_UI_LOCKUP)
endif(BUILD_uiblocktester)

kdevplatform_add_plugin(kdevcpplanguagesupport JSON kdevcppsupport.json
	SOURCES ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport
    kdevcpprpp
    kdevcppduchain
    kdevcppparser
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    ${KDE4_THREADWEAVER_LIBRARIES}

    KF5::IconThemes
    KF5::TextEditor
)

########### install files ###############

install(FILES kdevcppsupport.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevcppsupport)
