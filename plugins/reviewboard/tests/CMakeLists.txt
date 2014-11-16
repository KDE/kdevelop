include_directories(.. ${CMAKE_CURRENT_BINARY_DIR}/..)

set(reviewboardtest_SRCS
    main.cpp
    ../reviewpatchdialog.cpp
    ../reviewboardjobs.cpp
    ../debug.cpp
)

qt5_wrap_ui(reviewboardtest_SRCS ../reviewpatch.ui)

add_executable(reviewboardtest ${reviewboardtest_SRCS})
target_link_libraries(reviewboardtest KDev::Interfaces KF5::KIOWidgets)


