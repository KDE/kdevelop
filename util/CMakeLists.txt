include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(kdevclangutil OBJECT
    clangutils.cpp
    clangtypes.cpp
)
set_target_properties(kdevclangutil PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
