add_subdirectory(appwizard)
#add_subdirectory(documentview)
#add_subdirectory(codeview)
add_subdirectory(konsole)
add_subdirectory(qtdesigner)
#add_subdirectory(fileselector)
#add_subdirectory(valgrind)
add_subdirectory(filemanager)
add_subdirectory(snippet)

find_package(Commoncpp)
find_package(BoostLibraries)

if(COMMONCPP2_FOUND AND Boost_FOUND AND Boost_LIBS_FOUND)
	add_subdirectory(teamwork)
endif(COMMONCPP2_FOUND AND Boost_FOUND  AND Boost_LIBS_FOUND)

