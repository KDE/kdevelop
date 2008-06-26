add_subdirectory(tests)
add_subdirectory(config)

#
# TODO install kdevcheck headers
#

add_definitions(${KDE4_ENABLE_EXCEPTIONS})
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

set(TestBuild on)
if(TestBuild)
# expose all symbols
    add_definitions( -fvisibility=default )
endif(TestBuild)

include_directories(
    ${KDE4_INCLUDES}
    ${XtestPlugin_SOURCE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/shell
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/veritas
    ${KDEVPLATFORM_INCLUDE_DIR}/veritas/interfaces)

set(kdevcheck_SRCS
    outputparser.cpp
    testsuite.cpp)

qt4_automoc( ${kdevcheck_SRCS} )
kde4_add_library(kdevcheck SHARED ${kdevcheck_SRCS})
target_link_libraries(kdevcheck
    ${KDEVPLATFORM_VERITAS_LIBRARY}
    ${CHECK_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY})
install(TARGETS kdevcheck ${INSTALL_TARGETS_DEFAULT_ARGS} )

#########################################

set(kdevcheckview_SRCS checkview.cpp)
qt4_automoc(${kdevcheckview_SRCS})
kde4_add_plugin(kdevcheckview ${kdevcheckview_SRCS})
target_link_libraries(kdevcheckview
    kdevcheck
    ${KDEVPLATFORM_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY})
install(TARGETS kdevcheckview DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevcheck.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevcheck.rc DESTINATION ${DATA_INSTALL_DIR}/kdevcheck )

