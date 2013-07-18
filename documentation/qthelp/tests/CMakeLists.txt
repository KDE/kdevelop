add_definitions( -DQTHELP_FILES="\\"${CMAKE_CURRENT_SOURCE_DIR}/documentation\\"" )

set(qthelpplugintest_SRCS
    testqthelpplugin.cpp
    ../qthelpplugin.cpp
    ../qthelpproviderabstract.cpp
    ../qthelpprovider.cpp
    ../qthelpdocumentation.cpp
    ../qthelpqtdoc.cpp
    ../qthelp_config_shared.cpp
)

kde4_add_unit_test(qthelpplugintest ${qthelpplugintest_SRCS} )
target_link_libraries( qthelpplugintest ${QT_QTTEST_LIBRARY}     ${KDE4_KCMUTILS_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${QT_QTHELP_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_DOCUMENTATION_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})
