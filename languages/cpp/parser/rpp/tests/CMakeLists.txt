include_directories( ${KDevelop_SOURCE_DIR}/languages/cpp/parser ${CMAKE_CURRENT_SOURCE_DIR}/..  )

add_executable(pp main.cpp)
target_link_libraries(pp  ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} kdev4cpprpp)

