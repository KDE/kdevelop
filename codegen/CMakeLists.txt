add_library(kdevclangcodegen STATIC
    adaptsignatureaction.cpp
    adaptsignatureassistant.cpp
    simplerefactoring.cpp
    clangsignatureassistant.cpp
    codegenhelper.cpp
)
target_link_libraries(kdevclangcodegen
LINK_PRIVATE
    KF5::TextEditor
    KF5::ThreadWeaver
    KDev::Language
    kdevclangduchain
)
set_target_properties(kdevclangcodegen PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
