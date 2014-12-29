set(test_custommake_SRCS
    test_custommake.cpp
    ../makefileresolver.cpp
    ../helper.cpp
)

ecm_add_test(${test_custommake_SRCS}
    TEST_NAME test_custommake
    LINK_LIBRARIES
        Qt5::Test
        KDev::Tests
        KDev::Project
)
if(WIN32)
    target_include_directories(test_custommake PUBLIC ${KDEWIN_INCLUDES})
    target_link_libraries(test_custommake ${KDEWIN_LIBRARIES})
endif()
