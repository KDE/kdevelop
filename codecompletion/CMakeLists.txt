include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(kdevclangcodecompletion OBJECT
    model.cpp
    context.cpp
    navigationwidget.cpp
    overridecompletionhelper.cpp
)
set_target_properties(kdevclangcodecompletion PROPERTIES
    AUTOMOC TRUE
    POSITION_INDEPENDENT_CODE TRUE)
