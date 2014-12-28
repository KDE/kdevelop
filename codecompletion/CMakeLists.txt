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

generate_export_header(kdevqmljscompletion EXPORT_MACRO_NAME KDEVQMLJSCOMPLETION_EXPORT
                                           EXPORT_FILE_NAME codecompletionexport.h)

target_link_libraries(kdevqmljscompletion LINK_PRIVATE
    KF5::KDELibs4Support
    KDev::Language
    KDev::Interfaces
    KDev::Project
    kdevqmljsduchain
    kdevqtc-qmlsupport
)

install(TARGETS kdevqmljscompletion DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
