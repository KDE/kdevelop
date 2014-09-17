set(test_manpagemodel_SRCS
    test_manpagemodel.cpp
    ../manpagedocumentation.cpp
    ../manpagedocumentationwidget.cpp
    ../manpagemodel.cpp
    ../manpageplugin.cpp
)

add_executable(test_manpagemodel ${test_manpagemodel_SRCS} )
target_link_libraries(test_manpagemodel Qt5::Test
    KDev::Documentation
    KDev::Interfaces
    KDev::Tests
)
ecm_mark_as_test(test_manpagemodel)
add_test(test_manpagemodel test_manpagemodel)
