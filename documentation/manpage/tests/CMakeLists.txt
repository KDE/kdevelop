set(test_manpagemodel_SRCS
    test_manpagemodel.cpp
    ../manpagedocumentation.cpp
    ../manpagedocumentationwidget.cpp
    ../manpagemodel.cpp
    ../manpageplugin.cpp
)

ecm_add_test(${test_manpagemodel_SRCS}
    TEST_NAME test_manpagemodel
    LINK_LIBRARIES Qt5::Test KDev::Documentation KDev::Interfaces KDev::Tests)
