macro_optional_add_subdirectory(duchainviewer)
macro_optional_add_subdirectory(konsole)
add_subdirectory(filemanager)
add_subdirectory(standardoutputview)
macro_optional_add_subdirectory(snippet)
macro_optional_add_subdirectory(cvs)

find_package(Commoncpp)
find_package(BoostLibraries)
if(COMMONCPP2_FOUND AND Boost_FOUND AND Boost_LIBS_FOUND)
    macro_optional_add_subdirectory(teamwork)
endif(COMMONCPP2_FOUND AND Boost_FOUND  AND Boost_LIBS_FOUND)

find_package(SubversionLibrary)
if(SUBVERSION_FOUND)
    macro_optional_add_subdirectory(subversion)
endif(SUBVERSION_FOUND)


