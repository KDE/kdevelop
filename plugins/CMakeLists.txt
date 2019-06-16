# BEGIN: Analyzers
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

# 3rdparty/qtcreator-libs uses QT_CONFIG which was added for 5.8
if(NOT Qt5_VERSION VERSION_LESS "5.8.0")
    ecm_optional_add_subdirectory(qmljs)
else()
    message(WARNING "Will not build QML/JS plugin, needs Qt >= 5.8.0")
endif()

find_package(Clang 3.8)
set(clangSearchHint "")
if (NOT CLANG_FOUND)
    set(clangSearchHint "Please install a package providing libclang. Either pass -DLLVM_ROOT=/path/to/llvm-prefix or install the 'llvm-config' command-line utility for auto-detection.")
endif()
set_package_properties(Clang PROPERTIES
    DESCRIPTION "Clang libraries from the LLVM project. ${clangSearchHint}"
    PURPOSE "Used for KDevelop's C++/C support plugin."
    TYPE REQUIRED
)

if (CLANG_FOUND)
    if (NOT CLANG_CLANG_LIB)
        message(FATAL_ERROR "Could not find the Clang C library: libclang")
    endif()

    add_library(Clang::clang UNKNOWN IMPORTED)
    set_property(TARGET Clang::clang PROPERTY IMPORTED_LOCATION ${CLANG_CLANG_LIB})
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
if (KDevelop-PG-Qt_FOUND)
    ecm_optional_add_subdirectory(qmakemanager)
endif()
ecm_optional_add_subdirectory(genericprojectmanager)
# END: Project managers

# BEGIN: Runtimes
add_subdirectory(android)
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
add_subdirectory(patchreview)
add_subdirectory(problemreporter)
add_subdirectory(projectfilter)
add_subdirectory(projectmanagerview)
add_subdirectory(quickopen)
add_subdirectory(sourceformatter)
add_subdirectory(standardoutputview)
add_subdirectory(switchtobuddy)
add_subdirectory(testview)
add_subdirectory(scratchpad)
add_subdirectory(meson)
ecm_optional_add_subdirectory(classbrowser)
ecm_optional_add_subdirectory(executeplasmoid)
ecm_optional_add_subdirectory(ghprovider)
ecm_optional_add_subdirectory(kdeprovider)
ecm_optional_add_subdirectory(konsole)
if (Qt5QuickWidgets_FOUND)
    add_subdirectory(welcomepage)
endif()

find_package(OktetaKastenControllers 0.3.1 CONFIG)
set_package_properties(OktetaKastenControllers PROPERTIES
    PURPOSE "Required for building Okteta KDevelop plugin."
    URL "https://kde.org/"
    TYPE OPTIONAL)
if (OktetaKastenControllers_FOUND)
    find_package(KastenControllers CONFIG)

    set_package_properties(KastenControllers PROPERTIES
        PURPOSE "Required for building Okteta KDevelop plugin."
        URL "https://kde.org/"
        TYPE OPTIONAL)
endif()
if (OktetaKastenControllers_FOUND AND KastenControllers_FOUND)
    add_subdirectory(okteta)
endif()
# END: Others
