add_definitions(-DTRANSLATION_DOMAIN=\"kdevclang\")
add_definitions(${LLVM_CFLAGS})
include_directories(${CLANG_INCLUDE_DIRS})

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_definitions(
    -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_BYTEARRAY
)

# TODO: Move to kdevplatform
function(add_private_library target)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(KDEV_ADD_PRIVATE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    string(REPLACE "KDev" "" shortTargetName ${target})
    if (${shortTargetName} STREQUAL ${target})
        message(FATAL_ERROR "Target passed to add_private_library needs to start with \"KDev\", was \"${target}\"")
    endif()

    string(TOLOWER ${shortTargetName} shortTargetNameToLower)

    add_library(${target} SHARED ${KDEV_ADD_PRIVATE_SOURCES})
    generate_export_header(${target} EXPORT_FILE_NAME ${shortTargetNameToLower}export.h)
    set_target_properties(${target} PROPERTIES
        VERSION ${KDEV_PLUGIN_VERSION}
        SOVERSION ${KDEV_PLUGIN_VERSION}
    )
    install(TARGETS ${target} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
endfunction()

set(kdevclangprivate_SRCS
    clangsettings/clangsettingsmanager.cpp
    clangsettings/sessionsettings/sessionsettings.cpp

    codecompletion/completionhelper.cpp
    codecompletion/context.cpp
    codecompletion/includepathcompletioncontext.cpp
    codecompletion/model.cpp

    codegen/adaptsignatureaction.cpp
    codegen/adaptsignatureassistant.cpp
    codegen/codegenhelper.cpp
    codegen/clangrefactoring.cpp
    codegen/clangclasshelper.cpp
    codegen/sourcemanipulation.cpp

    duchain/builder.cpp
    duchain/clangdiagnosticevaluator.cpp
    duchain/clangducontext.cpp
    duchain/clanghelpers.cpp
    duchain/clangindex.cpp
    duchain/clangparsingenvironment.cpp
    duchain/clangparsingenvironmentfile.cpp
    duchain/clangpch.cpp
    duchain/clangproblem.cpp
    duchain/debugvisitor.cpp
    duchain/documentfinderhelpers.cpp
    duchain/duchainutils.cpp
    duchain/macrodefinition.cpp
    duchain/macronavigationcontext.cpp
    duchain/missingincludepathproblem.cpp
    duchain/navigationwidget.cpp
    duchain/parsesession.cpp
    duchain/todoextractor.cpp
    duchain/types/classspecializationtype.cpp
    duchain/unknowndeclarationproblem.cpp
    duchain/unsavedfile.cpp

    util/clangdebug.cpp
    util/clangtypes.cpp
    util/clangutils.cpp
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

ki18n_wrap_ui(kdevclangprivate_SRCS
    clangsettings/sessionsettings/sessionsettings.ui
)

kconfig_add_kcfg_files(kdevclangprivate_SRCS clangsettings/sessionsettings/sessionconfig.kcfgc)

add_private_library(KDevClangPrivate SOURCES ${kdevclangprivate_SRCS})
target_link_libraries(KDevClangPrivate
LINK_PRIVATE
    Qt5::Core
    KF5::TextEditor
    KF5::ThreadWeaver
    KDev::DefinesAndIncludesManager
    KDev::Util
LINK_PUBLIC
    KDev::Language
    KDev::Project
    KDev::Util
    Clang::clang
)

install(FILES duchain/gcc_compat.h DESTINATION ${KDE_INSTALL_DATADIR}/kdevclangsupport PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
install(DIRECTORY duchain/wrappedQtHeaders DESTINATION ${KDE_INSTALL_DATADIR}/kdevclangsupport
        DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_WRITE GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
        FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

set(kdevclangsupport_SRCS
    clangparsejob.cpp
    clangsupport.cpp
    clanghighlighting.cpp
)

qt5_add_resources(kdevclangsupport_SRCS kdevclangsupport.qrc)

kdevplatform_add_plugin(kdevclangsupport JSON kdevclangsupport.json SOURCES ${kdevclangsupport_SRCS})
target_link_libraries(kdevclangsupport
    KDevClangPrivate

    KF5::ThreadWeaver
    KF5::TextEditor
    KDev::Util
    KDev::Project
    KDev::DefinesAndIncludesManager
)

install(FILES kdevclang.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
