
include_directories(${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KDE4_INCLUDE_DIR}/threadweaver
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/editor
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
    ${CMAKE_SOURCE_DIR}/languages/cpp/cppduchain
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/language
    ${KDEVPLATFORM_INCLUDE_DIR}/language/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/language/backgroundparser
    ${KDEVPLATFORM_INCLUDE_DIR}/language/duchain
    ${KDEVPLATFORM_INCLUDE_DIR}/language/duchain/viewer
)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9007 ) 

add_subdirectory(parser)
add_subdirectory(cppduchain)
add_subdirectory(app_templates)
add_subdirectory(tests)
if(NOT WIN32)
    #TODO: port debugger to win32
    add_subdirectory(debugger)
endif(NOT WIN32)
########### next target ###############

set(kdevcpplanguagesupport_PART_SRCS
    cpplanguagesupport.cpp
    cppparsejob.cpp
    preprocessjob.cpp
    cpphighlighting.cpp
    cppcodecompletionmodel.cpp
    cppcodecompletionworker.cpp
    codecompletioncontext.cpp
    stringhelpers.cpp
    includepathresolver.cpp
    setuphelpers.cpp
    quickopen.cpp
    completionitem.cpp
    completionhelpers.cpp
)


ADD_EXECUTABLE(kdev_includepathresolver includepathresolver.cpp)

SET_TARGET_PROPERTIES( kdev_includepathresolver PROPERTIES COMPILE_FLAGS -DTEST )

TARGET_LINK_LIBRARIES( kdev_includepathresolver
${KDEVPLATFORM_INTERFACES_LIBRARY}  ${KDEVPLATFORM_PROJECT_LIBRARY}
${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} )

install(TARGETS kdev_includepathresolver ${INSTALL_TARGETS_DEFAULT_ARGS} )

kde4_add_plugin(kdevcpplanguagesupport ${kdevcpplanguagesupport_PART_SRCS})
target_link_libraries(kdevcpplanguagesupport ${QT_QTDESIGNER_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY} kdev4cppduchain kdev4cpprpp
    ${KDEVPLATFORM_PROJECT_LIBRARY} ${KDE4_THREADWEAVER_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARY} khtml
    ${KDE4_KDECORE_LIBS}
)

install(TARGETS kdevcpplanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES kdevcppsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevcppsupport.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcppsupport)
