include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(kdevclangcodecompletion OBJECT
    model.cpp
    context.cpp
    completionhelper.cpp
)
target_include_directories(kdevclangcodecompletion PRIVATE $<TARGET_PROPERTY:KF5::TextEditor,INTERFACE_INCLUDE_DIRECTORIES>)
set_target_properties(kdevclangcodecompletion PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
