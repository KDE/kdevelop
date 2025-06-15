remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

set(test_areaoperation_SRCS test_areaoperation.cpp areaprinter.cpp)
ecm_add_test(${test_areaoperation_SRCS}
    TEST_NAME test_areaoperation
    LINK_LIBRARIES Qt::Test KF6::I18n KF6::Parts KDev::Sublime)


set(test_areawalker_SRCS test_areawalker.cpp areaprinter.cpp)
ecm_add_test(${test_areawalker_SRCS}
    TEST_NAME test_areawalker
    LINK_LIBRARIES Qt::Test KF6::Parts KDev::Sublime)

ecm_add_test(test_view.cpp
    LINK_LIBRARIES Qt::Test KF6::Parts KDev::Sublime)

ecm_add_test(test_document.cpp
    LINK_LIBRARIES Qt::Test KF6::Parts KDev::Sublime)

ecm_add_test(test_controller.cpp
    LINK_LIBRARIES Qt::Test KF6::Parts KDev::Sublime)

ecm_add_test(test_viewactivation.cpp
    LINK_LIBRARIES Qt::Test KF6::I18n KF6::Parts KDev::Sublime)

ecm_add_test(test_aggregatemodel.cpp
    LINK_LIBRARIES Qt::Test KF6::Parts KDev::Sublime)

ecm_add_test(test_toolviewtoolbar.cpp
    LINK_LIBRARIES Qt::Test KF6::I18n KF6::Parts KDev::Sublime)
