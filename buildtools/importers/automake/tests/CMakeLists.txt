kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${CMAKE_SOURCE_DIR}/buildtools/importers/automake ${KDE4_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

if(KDE4_BUILD_TESTS)


########### next target ###############

set(runner_SRCS
    runner.cpp
)

kde4_add_executable(runner ${runner_SRCS})
target_link_libraries(runner ${KDE4_KDECORE_LIBS} kdevautomakeparser kio)


########### next target ###############

set(viewer_SRCS
    viewer.cpp
    viewer_main.cpp
)

kde4_automoc(${viewer_SRCS})

set(viewer_UI
    viewerbase.ui
)

kde4_add_ui3_files(viewer_SRCS ${viewer_UI})
kde4_add_executable(viewer ${viewer_SRCS})
target_link_libraries(viewer ${KDE4_KDECORE_LIBS} kdevautomakeparser kio)


########### next target ###############

set(testinterface_SRCS
    testinterface.cpp
)

kde4_automoc(${testinterface_SRCS})
kde4_add_executable(testinterface ${testinterface_SRCS})
target_link_libraries(testinterface ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdevautomakeparser kio)
add_test(kdevelop-automakeparsertest ${EXECUTABLE_OUTPUT_PATH}/testinterface)


endif(KDE4_BUILD_TESTS)


kde4_footer()
