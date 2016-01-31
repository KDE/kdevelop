ecm_add_test(test_projectmodel.cpp
    LINK_LIBRARIES Qt5::Test KDev::Interfaces KDev::Project KDev::Language KDev::Tests)

add_executable(projectmodelperformancetest
    projectmodelperformancetest.cpp
)
target_link_libraries(projectmodelperformancetest
    Qt5::Test
    Qt5::Gui
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Tests
)

add_executable(abstractfilemanagerpluginimporttest
    abstractfilemanagerpluginimporttest.cpp
)
target_link_libraries(abstractfilemanagerpluginimporttest
    KDev::Project
    KDev::Tests
    Qt5::QuickWidgets
)
