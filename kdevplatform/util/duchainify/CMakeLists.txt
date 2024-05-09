
add_executable(duchainify main.cpp)
target_link_libraries(duchainify
    KF6::KCMUtils
    KF6::TextEditor
    KDev::Interfaces
    KDev::Language
    KDev::Sublime
    KDev::Util
    KDev::OutputView
    KDev::Shell
    KDev::Tests
)

