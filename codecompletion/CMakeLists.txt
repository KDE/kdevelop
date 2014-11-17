add_library(kdevclangcodecompletion STATIC
    model.cpp
    context.cpp
    includepathcompletioncontext.cpp
    completionhelper.cpp
)
target_link_libraries(kdevclangcodecompletion
LINK_PRIVATE
    KF5::TextEditor
    KDev::Language
)
set_target_properties(kdevclangcodecompletion PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
