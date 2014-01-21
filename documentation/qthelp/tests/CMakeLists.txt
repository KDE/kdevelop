configure_file(testqthelpconfig.h.in testqthelpconfig.h)

set(qthelpplugintest_SRCS
    testqthelpplugin.cpp
    ../qthelpplugin.cpp
    ../qthelpproviderabstract.cpp
    ../qthelpprovider.cpp
    ../qthelpdocumentation.cpp
    ../qthelpqtdoc.cpp
    ../qthelp_config_shared.cpp
)

add_executable(qthelpplugintest ${qthelpplugintest_SRCS} )
target_link_libraries( qthelpplugintest Qt5::Test     ${KDE4_KCMUTILS_LIBS} KF5::KIOWidgets ${KDE4_KDEUI_LIBS} KF5::TextEditor ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

ecm_mark_as_test(qthelpplugintest)
add_test(qthelpplugintest qthelpplugintest)
