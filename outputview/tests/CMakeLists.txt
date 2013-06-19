set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro(OUTPUTVIEW_CREATE_TEST name)
    kde4_add_unit_test(${name} ${ARGN})
    target_link_libraries(${name}
                          ${QT_QTTEST_LIBRARY}
                          ${KDEVPLATFORM_TESTS_LIBRARIES}
                          kdevplatformoutputview
                         )
endmacro(OUTPUTVIEW_CREATE_TEST)

add_definitions( -DPROJECTS_SOURCE_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/projects\\"" )

outputview_create_test(outputmodeltest outputmodeltest.cpp)
outputview_create_test(filteringstrategytest filteringstrategytest.cpp)

