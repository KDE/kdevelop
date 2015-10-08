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
