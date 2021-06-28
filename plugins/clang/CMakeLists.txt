add_definitions(-DTRANSLATION_DOMAIN=\"kdevclang\")
add_definitions(${LLVM_CFLAGS})
include_directories(${CLANG_INCLUDE_DIRS})

set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_DL_LIBS})
check_cxx_source_compiles(
    "#include <dlfcn.h>\nint main() { Dl_info info; return dladdr(nullptr, &info); }"
    HAVE_DLFCN)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/libclang_include_path.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/libclang_include_path.h"
  @ONLY
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

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
    duchain/headerguardassistant.cpp

    util/clangdebug.cpp
    util/clangtypes.cpp
    util/clangutils.cpp
)
# dummy call to add the data to KDevelopCategories
# util/clangdebug.* cannot easily be generated with ecm_qt_declare_logging_category
# as the current code does not use Q_DECLARE_LOGGING_CATEGORY but instead
# has explicit code to tag KDEV_CLANG() as KDEVCLANGPRIVATE_EXPORT
# Keep in sync with util/clangdebug.*
declare_qt_logging_category(dummy_kdevclangprivate_SRCS
    TYPE PLUGIN
    HEADER dummy_debug.h
    IDENTIFIER KDEV_CLANG
    CATEGORY_BASENAME "clang"
    DESCRIPTION "clang-based language support"
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

ki18n_wrap_ui(kdevclangprivate_SRCS
    clangsettings/sessionsettings/sessionsettings.ui
)

kconfig_add_kcfg_files(kdevclangprivate_SRCS clangsettings/sessionsettings/sessionconfig.kcfgc)

kdevelop_add_private_library(KDevClangPrivate SOURCES ${kdevclangprivate_SRCS})
target_link_libraries(KDevClangPrivate
PUBLIC
    KDev::Language
    KDev::Project
    KDev::Util
    Clang::clang
PRIVATE
    Qt5::Core
    KF5::TextEditor
    KF5::ThreadWeaver
    KDev::DefinesAndIncludesManager
    KDev::Util
    KDev::Sublime
)

if (HAVE_DLFCN)
    target_link_libraries(KDevClangPrivate PRIVATE ${CMAKE_DL_LIBS})
endif()

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

kdevplatform_add_plugin(kdevclangsupport SOURCES ${kdevclangsupport_SRCS})
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
