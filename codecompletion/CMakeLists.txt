include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(completion_SRCS
    items/completionitem.cpp
    items/modulecompletionitem.cpp
    items/functioncalltipcompletionitem.cpp

    context.cpp
    model.cpp
    worker.cpp
    ../libs/qmljs/parser/qmljsgrammar.cpp
)

add_library(kdevqmljscompletion SHARED ${completion_SRCS})

target_link_libraries(kdevqmljscompletion LINK_PRIVATE
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdevqmljsduchain
    kdevqtc-qmlsupport
)

install(TARGETS kdevqmljscompletion DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
