add_subdirectory(tests)
add_subdirectory(config)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

set(TestBuild on)
if(TestBuild)
    # expose all symbols
    add_definitions( -fvisibility=default )
endif(TestBuild)

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/shell
    ${KDEVPLATFORM_INCLUDE_DIR}/project
    ${KDEVPLATFORM_INCLUDE_DIR}/project/interfaces
    ${KDEVPLATFORM_INCLUDE_DIR}/veritas
    ${KDEVPLATFORM_INCLUDE_DIR}/veritas/interfaces)

set(kdevqtest_SRCS
    qtestoutputparser.cpp
    qtestcase.cpp
    qtestcommand.cpp
    qtestsuite.cpp
    qtestregister.cpp)

qt4_automoc(${kdevqtest_SRCS})
kde4_add_library(kdevqtest SHARED ${kdevqtest_SRCS})
target_link_libraries(kdevqtest
    ${KDEVPLATFORM_VERITAS_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTXML_LIBRARY})
install(TARGETS kdevqtest ${INSTALL_TARGETS_DEFAULT_ARGS} )

set(kdevqtestview_SRCS qtestview.cpp)
qt4_automoc(${kdevqtestview_SRCS})
kde4_add_plugin(kdevqtestview ${kdevqtestview_SRCS})
target_link_libraries(kdevqtestview
    kdevqtest
    ${KDEVPLATFORM_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY})
install(TARGETS kdevqtestview DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevqtest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevqtest.rc DESTINATION ${DATA_INSTALL_DIR}/kdevqtest )
