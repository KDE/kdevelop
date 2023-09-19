
add_executable(duchainify main.cpp)
target_link_libraries(duchainify
    KF${KF_MAJOR_VERSION}::KCMUtils
    KF${KF_MAJOR_VERSION}::TextEditor
    KDev::Interfaces
    KDev::Language
    KDev::Sublime
    KDev::Util
    KDev::OutputView
    KDev::Shell
    KDev::Tests
)

