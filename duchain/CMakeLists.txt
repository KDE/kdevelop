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

    $<TARGET_OBJECTS:kdevclangutil>
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(kdevclangduchain LINK_PRIVATE
    Qt5::Core
)

target_link_libraries(kdevclangduchain LINK_PUBLIC
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${CLANG_CLANG_LIB}
)

install(TARGETS kdevclangduchain ${INSTALL_TARGETS_DEFAULT_ARGS})
