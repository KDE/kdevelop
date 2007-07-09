
add_definitions ( -DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include_directories(${KDEVPLATFORM_INCLUDE_DIR}/ ${KDEVPLATFORM_INCLUDE_DIR}/util
    ${CMAKE_SOURCE_DIR}/buildtools/importers/automake
    )

if(false)


########### next target ###############

set(runner_SRCS
    runner.cpp
)

kde4_add_test_executable(runner ${runner_SRCS})
target_link_libraries(runner ${KDE4_KDECORE_LIBS} kdevautomakeparser ${KDE4_KIO_LIBS})


########### next target ###############

set(viewer_SRCS
    viewer.cpp
    viewer_main.cpp
)



set(viewer_UI
    viewerbase.ui
)

kde4_add_ui3_files(viewer_SRCS ${viewer_UI})
kde4_add_test_executable(viewer ${viewer_SRCS})
target_link_libraries(viewer ${KDE4_KDECORE_LIBS} kdevautomakeparser ${KDE4_KIO_LIBS})


########### next target ###############

set(testinterface_SRCS
    testinterface.cpp
)


kde4_add_unit_test(testinterface TESTNAME kdevelop-testinterface ${testinterface_SRCS})
target_link_libraries(testinterface ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdevautomakeparser ${KDE4_KIO_LIBS})

endif(false)


