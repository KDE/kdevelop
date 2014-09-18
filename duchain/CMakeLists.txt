add_library(kdevqmljsduchain SHARED
    debug.cpp
    cache.cpp
    helper.cpp
    parsesession.cpp
    debugvisitor.cpp
    contextbuilder.cpp
    declarationbuilder.cpp
    expressionvisitor.cpp
    usebuilder.cpp
    qmljsducontext.cpp
    functiondeclaration.cpp
    functiontype.cpp

    navigation/declarationnavigationcontext.cpp
    navigation/navigationwidget.cpp

    frameworks/nodejs.cpp
)

target_link_libraries(kdevqmljsduchain LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
    KF5::I18n
)

target_link_libraries(kdevqmljsduchain LINK_PUBLIC
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdevqtc-qmlsupport
)

install(TARGETS kdevqmljsduchain ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
