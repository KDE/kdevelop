find_package(Clang 3.5 REQUIRED)

add_definitions(${LLVM_CFLAGS})
include_directories(${CLANG_INCLUDE_DIRS})

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${KDevelop_SOURCE_DIR}/languages/plugin
)

add_subdirectory(tests)

add_definitions(
    -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_BYTEARRAY
)

add_subdirectory(clangsettings)
add_subdirectory(duchain)
add_subdirectory(codecompletion)
add_subdirectory(codegen)
add_subdirectory(util)

set(kdevclangsupport_SRCS
    clangparsejob.cpp
    clangsupport.cpp
    clanghighlighting.cpp
)

kdevplatform_add_plugin(kdevclangsupport JSON kdevclangsupport.json SOURCES ${kdevclangsupport_SRCS})

target_link_libraries(kdevclangsupport
    kdevclangduchain
    kdevclangcodecompletion
    kdevclangcodegen
    kdevclangutil
    sessionsettings

    KF5::ThreadWeaver
    KF5::TextEditor
    KDev::Util
    KDev::Project
)

install(FILES kdevclangsupport.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevclangsupport)
