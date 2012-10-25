kde4_add_executable(qml-parser
    qml-parser.cpp
    ../duchain/parsesession.cpp
    ../duchain/debugvisitor.cpp
)
target_link_libraries(qml-parser ${QT_QTCORE_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} kdevqtc-qmlsupport)