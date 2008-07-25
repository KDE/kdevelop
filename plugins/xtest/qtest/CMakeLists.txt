add_subdirectory(tests)
add_subdirectory(config)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

set(TestBuild on)
if(TestBuild AND NOT WIN32)
    # expose all symbols
    add_definitions( -fvisibility=default )
endif(TestBuild AND NOT WIN32)

include_directories(
    config
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces # needed for core.h & documentcontroller.h
    ${KDEVPLATFORM_INCLUDE_DIR}/veritas
    )

set(kdevqtest_SRCS
    qtestoutputparser.cpp
    qtestcase.cpp
    qtestcommand.cpp
    qtestsuite.cpp
    qtestviewdata.cpp
    qtestregister.cpp
    qtestoutputmorpher.cpp
    config/qtestsettings.cpp
    outputview/qtestoutputmodel.cpp
    outputview/qtestoutputdelegate.cpp
    outputview/qtestoutputjob.cpp)

qt4_automoc(${kdevqtest_SRCS})
kde4_add_library(kdevqtest SHARED ${kdevqtest_SRCS})
target_link_libraries(kdevqtest
    ${KDEVPLATFORM_VERITAS_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARY}
    ${KDEVPLATFORM_UTIL_LIBRARY}
    ${KDEVPLATFORM_SUBLIME_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTXML_LIBRARY})
install(TARGETS kdevqtest ${INSTALL_TARGETS_DEFAULT_ARGS} )

set(kdevqtestview_SRCS qtestplugin.cpp)
qt4_automoc(${kdevqtestview_SRCS})
kde4_add_plugin(kdevqtestview ${kdevqtestview_SRCS})
target_link_libraries(kdevqtestview
    kdevqtest
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBS})

install(TARGETS kdevqtestview DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevqtest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevqtest.rc DESTINATION ${DATA_INSTALL_DIR}/kdevqtest )

