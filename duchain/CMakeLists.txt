add_library(kdevclangduchain SHARED
    parsesession.cpp
    clangdiagnosticevaluator.cpp
    clangducontext.cpp
    clangindex.cpp
    clangparsingenvironmentfile.cpp
    clangparsingenvironment.cpp
    clangproblem.cpp
    debugvisitor.cpp
    duchainutils.cpp
    tuduchain.cpp
    clangpch.cpp
    clanghelpers.cpp
    unknowndeclarationproblem.cpp
    macrodefinition.cpp
    missingincludepathproblem.cpp
    macronavigationcontext.cpp
    navigationwidget.cpp
    todoextractor.cpp
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

generate_export_header(kdevclangduchain EXPORT_FILE_NAME clangduchainexport.h)
target_link_libraries(kdevclangduchain
LINK_PRIVATE
    Qt5::Core
    kdevclangcodegen
    kdevclangcodecompletion
    kdevclangutil
LINK_PUBLIC
    KDev::Language
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${CLANG_CLANG_LIB}
)

install(TARGETS kdevclangduchain ${INSTALL_TARGETS_DEFAULT_ARGS})
