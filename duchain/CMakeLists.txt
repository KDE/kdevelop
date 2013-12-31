find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_CFLAGS})

include_directories(
    ${LLVM_INCLUDE_DIRS}
)

kde4_add_library(kdevclangduchain SHARED
    parsesession.cpp
    clangtypes.cpp
    debugvisitor.cpp
    buildduchainvisitor.cpp
)

target_link_libraries(kdevclangduchain LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
)

target_link_libraries(kdevclangduchain LINK_PUBLIC
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    # TODO cleanup FindClang.cmake and look for it there
    ${LLVM_LIBRARY_DIR}/libclang.so
)

install(TARGETS kdevclangduchain ${INSTALL_TARGETS_DEFAULT_ARGS})
