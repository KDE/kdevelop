# include current binary dir explicitly to make nodejs.h find duchainexport.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

generate_export_header(kdevqmljsduchain EXPORT_MACRO_NAME KDEVQMLJSDUCHAIN_EXPORT
                                        EXPORT_FILE_NAME duchainexport.h)

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
