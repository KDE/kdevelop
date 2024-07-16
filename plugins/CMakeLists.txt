# BEGIN: Analyzers
add_subdirectory(compileanalyzercommon)
add_subdirectory(clangtidy)
add_subdirectory(clazy)
add_subdirectory(cppcheck)
if(UNIX AND NOT (APPLE OR CYGWIN))
    add_subdirectory(heaptrack)
endif()
# END: Analyzers

# BEGIN: Debuggers
add_subdirectory(debuggercommon)
add_subdirectory(lldb)
add_subdirectory(gdb)
# END: Debuggers

# BEGIN: Documentation
find_package(Qt5Help CONFIG)
set_package_properties(Qt5Help PROPERTIES
    PURPOSE "The Help module for the Qt toolkit, needed for the qthelp plugin"
    URL "https://www.qt.io/"
    TYPE OPTIONAL)
if(Qt5Help_FOUND)
    ecm_optional_add_subdirectory(qthelp)
endif()
ecm_optional_add_subdirectory(manpage)
# END: Documentation

# BEGIN: Formatters
add_subdirectory(astyle)
add_subdirectory(customscript)
# END: Formatters

# BEGIN: Languages
ecm_optional_add_subdirectory(custom-definesandincludes)

# TODO: port qmljs to Qt6
#ecm_optional_add_subdirectory(qmljs)

find_package(Clang CONFIG)
set(clangSearchHint "")
if (NOT Clang_FOUND)
    set(clangSearchHint "
        Please install a package providing libclang.
        You can pass -DClang_DIR=/path/to/lib/cmake/clang to cmake.
        You may need to remove stale CLANG_BUILTIN_DIR and LLVM_DIR CMake cache variables.")
endif()
set_package_properties(Clang PROPERTIES
    DESCRIPTION "Clang libraries from the LLVM project, required version >= 6.0${clangSearchHint}"
    PURPOSE "Used for KDevelop's C++/C support plugin."
    TYPE REQUIRED
)

if (Clang_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "  Using LLVMConfig.cmake in:  ${LLVM_DIR}")
    message(STATUS "  Using ClangConfig.cmake in: ${Clang_DIR}")

    if (NOT TARGET Clang::libclang)
        add_library(Clang::libclang ALIAS libclang)
    endif()
    ecm_optional_add_subdirectory(clang)
endif()
# END: Languages

# BEGIN: Project builders
add_subdirectory(makebuilder)
add_subdirectory(ninjabuilder)
ecm_optional_add_subdirectory(cmakebuilder)
if (KDevelop-PG-Qt_FOUND)
    ecm_optional_add_subdirectory(qmakebuilder)
endif()
# END: Project builders

# BEGIN: Project managers
ecm_optional_add_subdirectory(cmake)
ecm_optional_add_subdirectory(custommake)
ecm_optional_add_subdirectory(custom-buildsystem)
add_subdirectory(meson)
if (KDevelop-PG-Qt_FOUND)
    ecm_optional_add_subdirectory(qmakemanager)
endif()
ecm_optional_add_subdirectory(genericprojectmanager)
# END: Project managers

# BEGIN: Runtimes
add_subdirectory(android)
add_subdirectory(craft)
if (UNIX)
    add_subdirectory(docker)
    add_subdirectory(flatpak)
endif()
# END: Runtimes

# BEGIN: VCS
ecm_optional_add_subdirectory(bazaar)
ecm_optional_add_subdirectory(git)
ecm_optional_add_subdirectory(perforce)

find_package(SubversionLibrary)
set_package_properties(SubversionLibrary PROPERTIES
    PURPOSE "Support for Subversion integration"
    URL "https://subversion.apache.org/"
    TYPE OPTIONAL)

if(SubversionLibrary_FOUND)
    ecm_optional_add_subdirectory(subversion)
endif()

add_subdirectory(vcschangesview)
# END: VCS

# BEGIN: Others
add_subdirectory(appwizard)
add_subdirectory(codeutils)
add_subdirectory(contextbrowser)
add_subdirectory(documentswitcher)
add_subdirectory(documentview)
add_subdirectory(execute)
add_subdirectory(executescript)
add_subdirectory(externalscript)
add_subdirectory(filemanager)
add_subdirectory(filetemplates)
add_subdirectory(grepview)
add_subdirectory(openwith)
add_subdirectory(outlineview)
# TODO: port to libkomparediff2/qt6
#add_subdirectory(patchreview)
add_subdirectory(problemreporter)
add_subdirectory(projectfilter)
add_subdirectory(projectmanagerview)
add_subdirectory(quickopen)
add_subdirectory(sourceformatter)
add_subdirectory(standardoutputview)
add_subdirectory(switchtobuddy)
add_subdirectory(testview)
add_subdirectory(scratchpad)
ecm_optional_add_subdirectory(classbrowser)
ecm_optional_add_subdirectory(executeplasmoid)
ecm_optional_add_subdirectory(ghprovider)
ecm_optional_add_subdirectory(konsole)
add_subdirectory(welcomepage)

# TODO: Okteta Qt6 port not yet complete, ETA 10/2024 or later.
# Once available, adapt code & reenable again. For now prevent finding the Qt5 version.
set(CMAKE_DISABLE_FIND_PACKAGE_OktetaKastenControllers ON)
set(CMAKE_DISABLE_FIND_PACKAGE_KastenControllers ON)

find_package(OktetaKastenControllers 0.4.0 CONFIG)
set_package_properties(OktetaKastenControllers PROPERTIES
    PURPOSE "Required for building Okteta KDevelop plugin."
    URL "https://kde.org/"
    TYPE OPTIONAL)
if (OktetaKastenControllers_FOUND)
    find_package(KastenControllers 0.4.0 CONFIG)

    set_package_properties(KastenControllers PROPERTIES
        PURPOSE "Required for building Okteta KDevelop plugin."
        URL "https://kde.org/"
        TYPE OPTIONAL)
endif()
if (OktetaKastenControllers_FOUND AND KastenControllers_FOUND)
    add_subdirectory(okteta)
endif()
# END: Others
