include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(kdevclangcodecompletion STATIC
    model.cpp
    context.cpp
    completionhelper.cpp
)
target_link_libraries(kdevclangcodecompletion
LINK_PRIVATE
    KF5::TextEditor
    KDev::Language
)
set_target_properties(kdevclangcodecompletion PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
