project(kdev-clang)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
find_package(KDE4 REQUIRED)
find_package(KDevPlatform 1.5.60 REQUIRED)
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_CFLAGS})


set(VERSION_MAJOR 1)
set(VERSION_MINOR 5)
set(VERSION_PATCH 60)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

enable_testing()
set(KDE4_BUILD_TESTS "ON" CACHE "BOOL" "Enable building of tests" FORCE )

include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_subdirectory(duchain)

kde4_add_plugin(kdevclanglanguagesupport
    clangparsejob.cpp
    kdevclangplugin.cpp
)
target_link_libraries(kdevclanglanguagesupport
    kdevclangduchain

    ${KDE4_THREADWEAVER_LIBS}
)

install(FILES kdevclangsupport.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kdevclanglanguagesupport DESTINATION ${PLUGIN_INSTALL_DIR})
