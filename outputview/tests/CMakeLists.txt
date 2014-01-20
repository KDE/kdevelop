macro(OUTPUTVIEW_CREATE_TEST name)
    add_executable(${name} ${ARGN})
    ecm_mark_as_test(${name})
    target_link_libraries(${name}
                          Qt5::Test
                          ${KDEVPLATFORM_TESTS_LIBRARIES}
                          kdevplatformoutputview
                         )
endmacro(OUTPUTVIEW_CREATE_TEST)

add_definitions( -DPROJECTS_SOURCE_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/projects\\"" )

outputview_create_test(outputmodeltest outputmodeltest.cpp)
outputview_create_test(filteringstrategytest filteringstrategytest.cpp)

