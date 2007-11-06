macro_optional_add_subdirectory(duchainviewer)
macro_optional_add_subdirectory(konsole)
add_subdirectory(filemanager)
add_subdirectory(projectmanagerview)
add_subdirectory(genericprojectmanager)
add_subdirectory(standardoutputview)
add_subdirectory(quickopen)
macro_optional_add_subdirectory(snippet)
macro_optional_add_subdirectory(cvs)
add_subdirectory(classbrowser)
add_subdirectory(problemreporter)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_package(Commoncpp)
    find_package(BoostLibraries)

    if(COMMONCPP2_FOUND AND Boost_FOUND AND Boost_LIBS_FOUND)
        macro_optional_add_subdirectory(teamwork)
    endif(COMMONCPP2_FOUND AND Boost_FOUND  AND Boost_LIBS_FOUND)
endif(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)

find_package(SubversionLibrary)
macro_log_feature( SUBVERSION_FOUND "Subversion" "Support for Subversion integration" "http://subversion.tigris.org" FALSE "1.3.0" )
if(SUBVERSION_FOUND)
    macro_optional_add_subdirectory(subversion)
endif(SUBVERSION_FOUND)

macro_display_feature_log()


