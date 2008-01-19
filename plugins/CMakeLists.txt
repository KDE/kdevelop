
macro_optional_add_subdirectory(duchainviewer)
macro_optional_add_subdirectory(konsole)
add_subdirectory(filemanager)
add_subdirectory(projectmanagerview)
add_subdirectory(genericprojectmanager)
add_subdirectory(standardoutputview)
add_subdirectory(quickopen)
add_subdirectory(usehighlight)
macro_optional_add_subdirectory(snippet)
macro_optional_add_subdirectory(cvs)
add_subdirectory(classbrowser)
add_subdirectory(problemreporter)
add_subdirectory(execute)

macro_optional_find_package(Commoncpp)
macro_optional_find_package(Boost COMPONENTS thread serialization )
if(COMMONCPP2_FOUND AND Boost_FOUND)
    macro_optional_add_subdirectory(teamwork)
endif(COMMONCPP2_FOUND AND Boost_FOUND)
macro_log_feature( COMMONCPP2_FOUND "Common C++2" "Support for Teamwork plugin integration" "http://ftp.gnu.org/pub/gnu/commoncpp/" FALSE "1.5.9" "The commoncpp2 libraries are needed for the Teamwork plugin" )
macro_log_feature( Boost_FOUND "Boost" "Support for Teamwork plugin integration" "http://www.boost.org/" FALSE "1.34.0" "The boost C++ libraries are needed for the Teamwork plugin" )

macro_optional_find_package(SvnCpp)
macro_optional_find_package(SubversionLibrary)
macro_log_feature( SVNCPP_FOUND "SvnCpp" "Support for Subversion integration" "http://rapidsvn.tigris.org" FALSE "0.9.4" "The rapidsvn C++ library is needed for the Subversion support")
macro_log_feature( SUBVERSION_FOUND "Subversion" "Support for Subversion integration" "http://subversion.tigris.org" FALSE "1.3.0" "The subversion libraries are needed for the Subversion support" )
if(SUBVERSION_FOUND AND SVNCPP_FOUND)
    macro_optional_add_subdirectory(subversion)
endif(SUBVERSION_FOUND AND SVNCPP_FOUND)

macro_display_feature_log()


