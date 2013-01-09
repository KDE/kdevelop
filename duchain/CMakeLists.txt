kde4_add_library(kdevqmljsduchain SHARED
    parsesession.cpp
    debugvisitor.cpp
    contextbuilder.cpp
    declarationbuilder.cpp
    typebuilder.cpp
    debugvisitor.cpp
)

target_link_libraries(kdevqmljsduchain
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    kdevqtc-qmlsupport
)

target_link_libraries(kdevqmljsduchain LINK_INTERFACE_LIBRARIES
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    kdevqtc-qmlsupport
)

install(TARGETS kdevqmljsduchain ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
