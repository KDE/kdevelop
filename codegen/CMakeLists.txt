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
set_target_properties(kdevclangcodegen PROPERTIES
    AUTOMOC TRUE
    POSITION_INDEPENDENT_CODE TRUE)
