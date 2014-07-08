kde4_add_library(kdevqmljsduchain SHARED
    helper.cpp
    parsesession.cpp
    debugvisitor.cpp
    contextbuilder.cpp
    declarationbuilder.cpp
    expressionvisitor.cpp
    usebuilder.cpp
    qmljsducontext.cpp
    functiondeclaration.cpp

    navigation/declarationnavigationcontext.cpp
    navigation/navigationwidget.cpp
)

target_link_libraries(kdevqmljsduchain LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
)

target_link_libraries(kdevqmljsduchain LINK_PUBLIC
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    kdevqtc-qmlsupport
)

install(TARGETS kdevqmljsduchain ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
