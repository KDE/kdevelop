#@todo: this file needs to be worked over

include_directories( 
	${CMAKE_SOURCE_DIR}/kompare/libdialogpages 
	${CMAKE_SOURCE_DIR}/kompare/komparepart 
	${CMAKE_SOURCE_DIR}/kompare/interfaces  )

########### next target ###############

set(diff2_PART_SRCS
	kompareprocess.cpp 
	komparemodellist.cpp 
	diffmodellist.cpp 
	diffmodel.cpp 
	difference.cpp 
	diffhunk.cpp 
	levenshteintable.cpp 
	parser.cpp 
	parserbase.cpp 
	cvsdiffparser.cpp 
	diffparser.cpp 
	perforceparser.cpp
	diffsettings.cpp
	settingsbase.cpp )

kde4_automoc(${diff2_PART_SRCS})

kde4_add_library(diff2 ${diff2_PART_SRCS})

target_link_libraries(diff2  ${KDE4_KDEUI_LIBS} )

set_target_properties(diff2 PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install(TARGETS diff2 DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############


SET(CMAKE_CXX_FLAGS -fPIC)
SET(CMAKE_C_FLAGS -fPIC)
