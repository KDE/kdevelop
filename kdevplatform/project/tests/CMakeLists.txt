remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

ecm_add_test(test_projectmodel.cpp
    LINK_LIBRARIES Qt::Test KDev::Interfaces KDev::Project KDev::Language KDev::Tests)

add_executable(projectmodelperformancetest
    projectmodelperformancetest.cpp
)
target_link_libraries(projectmodelperformancetest
    Qt::Test
    Qt::Gui
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
    Qt::QuickWidgets
)

add_executable(abstractfilemanagerpluginimportbenchmark
    abstractfilemanagerpluginimportbenchmark.cpp
)
ecm_mark_nongui_executable(abstractfilemanagerpluginimportbenchmark)
target_link_libraries(abstractfilemanagerpluginimportbenchmark
    KDev::Project
    KDev::Tests
)
