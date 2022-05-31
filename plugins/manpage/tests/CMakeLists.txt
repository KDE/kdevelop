remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(test_manpagemodel_SRCS
    test_manpagemodel.cpp
    ../manpagedocumentation.cpp
    ../manpagedocumentationwidget.cpp
    ../manpagemodel.cpp
    ../manpageplugin.cpp
    ${kdevmanpage_LOG_SRCS}
)

if(Qt5_VERSION VERSION_LESS "5.11.0")
    set(test_manpagemodel_extra_link_libs KDev::Tests)
endif()
ecm_add_test(${test_manpagemodel_SRCS}
    TEST_NAME test_manpagemodel
    LINK_LIBRARIES
        Qt5::Test
        KDev::Documentation
        KDev::Interfaces
        KDev::Language
        ${test_manpagemodel_extra_link_libs}
)
