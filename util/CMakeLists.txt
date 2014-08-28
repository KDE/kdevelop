include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(kdevclangutil OBJECT
    clangdebug.cpp
    clangutils.cpp
    clangtypes.cpp
)
target_include_directories(kdevclangutil PRIVATE $<TARGET_PROPERTY:KF5::TextEditor,INTERFACE_INCLUDE_DIRECTORIES>)
set_target_properties(kdevclangutil PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
