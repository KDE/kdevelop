project(cpp)
include(CppBuildConfig.cmake)

include_directories(
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    ${CMAKE_SOURCE_DIR}/languages/cpp/cppduchain
)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9007 )

add_subdirectory(parser)
add_subdirectory(cppduchain)
add_subdirectory(veritas)
add_subdirectory(tests)
########### next target ###############

set(kdevcpplanguagesupport_PART_SRCS
    cpplanguagesupport.cpp
    includepathcomputer.cpp
    cppparsejob.cpp
    preprocessjob.cpp
    cpphighlighting.cpp
    codecompletion/model.cpp
    codecompletion/worker.cpp
    codecompletion/context.cpp
    stringhelpers.cpp
    includepathresolver.cpp
    setuphelpers.cpp
    quickopen.cpp
    codecompletion/item.cpp
    codecompletion/helpers.cpp
    codecompletion/missingincludeitem.cpp
    codecompletion/implementationhelperitem.cpp
    codecompletion/missingincludemodel.cpp

    codegen/cppnewclass.cpp
    codegen/simplerefactoring.cpp
    codegen/progressdialogs.cpp
    codegen/codeassistant.cpp
    codegen/signatureassistant.cpp
    codegen/unresolvedincludeassistant.cpp
)

kde4_add_ui_files(kdevcpplanguagesupport_PART_SRCS codegen/ui/custom_include_paths.ui)

option(BUILD_kdev_includepathresolver "Build the includepath resolver debugging tool" OFF)
if(BUILD_kdev_includepathresolver)
  ADD_EXECUTABLE(kdev_includepathresolver includepathresolver.cpp)
  SET_TARGET_PROPERTIES( kdev_includepathresolver PROPERTIES COMPILE_FLAGS -DTEST )
  TARGET_LINK_LIBRARIES( kdev_includepathresolver
  ${KDEVPLATFORM_INTERFACES_LIBRARY}  ${KDEVPLATFORM_PROJECT_LIBRARY}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDEVPLATFORM_LANGUAGE_LIBRARY} )
  install(TARGETS kdev_includepathresolver ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif(BUILD_kdev_includepathresolver)


kde4_add_plugin(kdevcpplanguagesupport ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport
    kdev4cppduchain
    kdev4cpprpp
    kdev4cppparser
    veritascpp
    ${QT_QTDESIGNER_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
)

install(TARGETS kdevcpplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevcppsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevcppsupport.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcppsupport)
