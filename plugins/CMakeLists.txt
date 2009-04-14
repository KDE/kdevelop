macro_optional_add_subdirectory(konsole)
add_subdirectory(filemanager)
#add_subdirectory(vcscommon)
add_subdirectory(appwizard)
add_subdirectory(projectmanagerview)
add_subdirectory(genericprojectmanager)
add_subdirectory(standardoutputview)
add_subdirectory(quickopen)
add_subdirectory(contextbrowser)
macro_optional_add_subdirectory(snippet)
macro_optional_add_subdirectory(cvs)
add_subdirectory(problemreporter)
add_subdirectory(execute)
add_subdirectory(sourceformatter)
add_subdirectory(documentswitcher)

macro_optional_find_package( Boost 1.35.0 )
macro_log_feature( Boost_FOUND "Boost" "Boost libraries for enabling the classbrowser" "http://www.boost.org" FALSE "1.35.0" "The boost libraries are needed to build the Class Browser" )
if(Boost_FOUND)
  add_subdirectory(classbrowser)
endif(Boost_FOUND)

macro_optional_find_package(SubversionLibrary)
macro_log_feature( SUBVERSION_FOUND "Subversion" "Support for Subversion integration" "http://subversion.tigris.org" FALSE "1.3.0" "The subversion libraries are needed for the Subversion support" )

if(SUBVERSION_FOUND)
    macro_optional_add_subdirectory(subversion)
endif(SUBVERSION_FOUND)

macro_optional_add_subdirectory(git)
macro_optional_add_subdirectory(mercurial)

macro_display_feature_log()

