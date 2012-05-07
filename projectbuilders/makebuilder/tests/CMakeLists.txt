set(testmakeoutputmodel_SRC test_makeoutputmodel.cpp ../makeoutputmodel.cpp ../outputfilters.cpp)

kde4_add_unit_test(testmakeoutputmodel ${testmakeoutputmodel_SRC})
target_link_libraries(testmakeoutputmodel ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES})
