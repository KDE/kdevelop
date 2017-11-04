
add_executable(duchainify main.cpp)
target_link_libraries(duchainify
    KF5::KCMUtils
    KF5::TextEditor
    KDev::Interfaces
    KDev::Language
    KDev::Sublime
    KDev::Util
    KDev::OutputView
    KDev::Shell
    KDev::Tests
)

