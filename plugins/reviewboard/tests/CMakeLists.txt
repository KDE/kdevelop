include_directories(.. ${CMAKE_CURRENT_BINARY_DIR}/..)

set(reviewboardtest_SRCS
    main.cpp
    ../reviewpatchdialog.cpp
    ../reviewboardjobs.cpp
)

kde4_add_ui_files(reviewboardtest_SRCS ../reviewpatch.ui)

kde4_add_executable(reviewboardtest ${reviewboardtest_SRCS})
target_link_libraries(reviewboardtest ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARIES})


