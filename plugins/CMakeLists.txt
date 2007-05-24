add_subdirectory(appwizard)
#add_subdirectory(documentview)
#add_subdirectory(codeview)
macro_optional_add_subdirectory(konsole)
macro_optional_add_subdirectory(qtdesigner)
#add_subdirectory(fileselector)
#add_subdirectory(valgrind)
add_subdirectory(filemanager)
macro_optional_add_subdirectory(snippet)

find_package(Commoncpp)
find_package(BoostLibraries)

if(COMMONCPP2_FOUND AND Boost_FOUND AND Boost_LIBS_FOUND)
	macro_optional_add_subdirectory(teamwork)
endif(COMMONCPP2_FOUND AND Boost_FOUND  AND Boost_LIBS_FOUND)

