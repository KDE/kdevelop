declare_qt_logging_category(kdevqmljsduchain_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER KDEV_QMLJS_DUCHAIN
    CATEGORY_BASENAME "qmljs.duchain"
    DESCRIPTION "qmljs duchain"
)
add_library(kdevqmljsduchain STATIC
    ${kdevqmljsduchain_LOG_SRCS}
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
target_link_libraries(kdevqmljsduchain
    PUBLIC
        kdevqtc-qmlsupport
        KDev::Language
        KDev::Util
        KDev::Project
    PRIVATE
        KF${KF_MAJOR_VERSION}::I18n
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
