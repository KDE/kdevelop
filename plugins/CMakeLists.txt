ecm_optional_add_subdirectory(konsole)
add_subdirectory(filemanager)
add_subdirectory(appwizard)
add_subdirectory(projectmanagerview)
add_subdirectory(genericprojectmanager)
add_subdirectory(standardoutputview)
add_subdirectory(documentview)
add_subdirectory(quickopen)
add_subdirectory(executescript)
add_subdirectory(contextbrowser)
add_subdirectory(snippet)
ecm_optional_add_subdirectory(cvs)
add_subdirectory(problemreporter)
add_subdirectory(execute)
add_subdirectory(externalscript)
add_subdirectory(documentswitcher)
add_subdirectory(patchreview)
add_subdirectory(openwith)
add_subdirectory(grepview)
add_subdirectory(pastebin)
add_subdirectory(codeutils)
add_subdirectory(git)
add_subdirectory(vcschangesview)
add_subdirectory(templatemanager)
add_subdirectory(filetemplates)
add_subdirectory(testview)
add_subdirectory(switchtobuddy)
add_subdirectory(projectfilter)

ecm_optional_add_subdirectory(dashboard)

set(Boost_ADDITIONAL_VERSIONS 1.39.0 1.39)
find_package(Boost 1.35.0)
set_package_properties(Boost PROPERTIES
    PURPOSE "Boost libraries for enabling the classbrowser"
    URL "http://www.boost.org"
    TYPE OPTIONAL)
if(Boost_FOUND)
    ecm_optional_add_subdirectory(classbrowser)
endif(Boost_FOUND)

find_package(SubversionLibrary)
set_package_properties(SubversionLibrary PROPERTIES
    PURPOSE "Support for Subversion integration"
    URL "http://subversion.tigris.org"
    TYPE OPTIONAL)

if(SUBVERSION_FOUND)
    ecm_optional_add_subdirectory(subversion)
endif(SUBVERSION_FOUND)

if(QJSON_FOUND)
    ecm_optional_add_subdirectory(reviewboard)
endif(QJSON_FOUND)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
