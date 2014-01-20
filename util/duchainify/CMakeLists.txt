
add_executable(duchainify main.cpp)
target_link_libraries(duchainify Qt5::Core KF5::KCMUtils KF5::TextEditor KF5::ThreadWeaver
    ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}  sublime kdevplatformutil kdevplatformoutputview kdevplatformshell kdevplatformtests)

