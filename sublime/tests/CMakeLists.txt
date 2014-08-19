set( test_areaoperation_SRCS test_areaoperation.cpp areaprinter.cpp )
add_executable(test_areaoperation ${test_areaoperation_SRCS})
add_test(test_areaoperation test_areaoperation)
ecm_mark_as_test(test_areaoperation)
target_link_libraries(test_areaoperation Qt5::Test KF5::Parts KDev::Sublime )


set( test_areawalker_SRCS test_areawalker.cpp areaprinter.cpp )
add_executable(test_areawalker ${test_areawalker_SRCS})
add_test(test_areawalker test_areawalker)
ecm_mark_as_test(test_areawalker)
target_link_libraries(test_areawalker Qt5::Test KF5::Parts KDev::Sublime )


set( test_view_SRCS test_view.cpp )
add_executable(test_view ${test_view_SRCS})
add_test(test_view test_view)
ecm_mark_as_test(test_view)
target_link_libraries(test_view Qt5::Test KF5::Parts KDev::Sublime )


set( test_document_SRCS test_document.cpp )
add_executable(test_document ${test_document_SRCS})
add_test(test_document test_document)
ecm_mark_as_test(test_document)
target_link_libraries(test_document Qt5::Test KF5::Parts KDev::Sublime )


set( test_controller_SRCS test_controller.cpp )
add_executable(test_controller ${test_controller_SRCS})
add_test(test_controller test_controller)
ecm_mark_as_test(test_controller)
target_link_libraries(test_controller Qt5::Test KF5::Parts KDev::Sublime )


set( test_viewactivation_SRCS test_viewactivation.cpp )
add_executable(test_viewactivation ${test_viewactivation_SRCS})
add_test(test_viewactivation test_viewactivation)
ecm_mark_as_test(test_viewactivation)
target_link_libraries(test_viewactivation Qt5::Test KF5::Parts KDev::Sublime )


set( test_aggregatemodel_SRCS test_aggregatemodel.cpp )
add_executable(test_aggregatemodel ${test_aggregatemodel_SRCS})
add_test(test_aggregatemodel test_aggregatemodel)
ecm_mark_as_test(test_aggregatemodel)
target_link_libraries(test_aggregatemodel Qt5::Test KF5::Parts KDev::Sublime KDev::Tests )

set( test_toolviewtoolbar_SRCS test_toolviewtoolbar.cpp )
add_executable(test_toolviewtoolbar ${test_toolviewtoolbar_SRCS})
add_test(test_toolviewtoolbar test_toolviewtoolbar)
ecm_mark_as_test(test_toolviewtoolbar)
target_link_libraries(test_toolviewtoolbar Qt5::Test KF5::Parts KDev::Sublime )

