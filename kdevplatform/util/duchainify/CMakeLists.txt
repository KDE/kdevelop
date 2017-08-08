
add_executable(duchainify main.cpp)
target_link_libraries(duchainify Qt5::Core KF5::KCMUtils KF5::TextEditor KF5::ThreadWeaver
    KDev::Interfaces KDev::Language  KDev::Sublime KDev::Util KDev::OutputView KDev::Shell KDev::Tests)

