include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(kdevclangutil STATIC
    clangdebug.cpp
    clangutils.cpp
    clangtypes.cpp
)
target_link_libraries(kdevclangutil
LINK_PRIVATE
    KDev::Util
    KF5::TextEditor)
set_target_properties(kdevclangutil PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
