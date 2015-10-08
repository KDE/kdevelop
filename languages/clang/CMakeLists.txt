project(kdev-clang)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

find_package(Clang 3.5)
set_package_properties(Clang PROPERTIES
    TYPE REQUIRED
    PURPOSE "LibClang is a high level C interface to clang"
    URL "http://clang.llvm.org/")

add_definitions(${LLVM_CFLAGS})
include_directories(${CLANG_INCLUDE_DIR})

# Turn off missing-field-initializers warning for GCC to avoid noise from false positives with empty {}
# See discussion: http://mail.kde.org/pipermail/kdevelop-devel/2014-February/046910.html
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

enable_testing()

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

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
