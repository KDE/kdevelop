kde4_add_library(kdevclangduchain SHARED
    parsesession.cpp
    clangtypes.cpp
    debugvisitor.cpp
    buildduchainvisitor.cpp
    usebuilder.cpp
    typebuilder.cpp
)

target_link_libraries(kdevclangduchain LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
)

target_link_libraries(kdevclangduchain LINK_PUBLIC
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${CLANG_CLANG_LIB}
)

install(TARGETS kdevclangduchain ${INSTALL_TARGETS_DEFAULT_ARGS})
