include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(kdevclangcodegen OBJECT
    adaptsignatureaction.cpp
    adaptsignatureassistant.cpp
    simplerefactoring.cpp
    clangsignatureassistant.cpp
    codegenhelper.cpp
)
target_include_directories(kdevclangcodegen PRIVATE
    $<TARGET_PROPERTY:KF5::TextEditor,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ThreadWeaver,INTERFACE_INCLUDE_DIRECTORIES>)
set_target_properties(kdevclangcodegen PROPERTIES
    AUTOMOC TRUE
    POSITION_INDEPENDENT_CODE TRUE)
