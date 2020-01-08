set(completion_SRCS
    items/completionitem.cpp
    items/modulecompletionitem.cpp
    items/functioncalltipcompletionitem.cpp

    context.cpp
    model.cpp
    worker.cpp
    #../libs/qmljs/parser/qmljsgrammar.cpp
)

add_library(kdevqmljscompletion STATIC ${completion_SRCS})

generate_export_header(kdevqmljscompletion EXPORT_MACRO_NAME KDEVQMLJSCOMPLETION_EXPORT
                                           EXPORT_FILE_NAME codecompletionexport.h)

target_link_libraries(kdevqmljscompletion
    PRIVATE
        kdevqmljsduchain
        kdevqtc-qmlsupport
        KDev::Language
        KDev::Interfaces
        KDev::Project
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
