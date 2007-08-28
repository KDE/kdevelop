macro_optional_add_subdirectory(duchainviewer)
macro_optional_add_subdirectory(konsole)
add_subdirectory(filemanager)
add_subdirectory(projectmanagerview)
add_subdirectory(genericprojectmanager)
add_subdirectory(standardoutputview)
macro_optional_add_subdirectory(snippet)
macro_optional_add_subdirectory(cvs)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_package(Commoncpp)
    find_package(BoostLibraries)
    if(COMMONCPP2_FOUND AND Boost_FOUND AND Boost_LIBS_FOUND)
        macro_optional_add_subdirectory(teamwork)
    endif(COMMONCPP2_FOUND AND Boost_FOUND  AND Boost_LIBS_FOUND)
endif(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)

find_package(SubversionLibrary)
if(SUBVERSION_FOUND)
    macro_optional_add_subdirectory(subversion)
endif(SUBVERSION_FOUND)


