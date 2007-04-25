#@todo: this file needs to be worked over

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


#this is needed until somebody finds a way to not produce a
#13 Meg static lib in debug build.
SET(CMAKE_BUILD_TYPE release)

kde4_automoc(${diff2_PART_SRCS})

kde4_add_library(diff2 ${diff2_PART_SRCS})

target_link_libraries(diff2  ${KDE4_KDEUI_LIBS} )

set_target_properties(diff2 PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install(TARGETS diff2 DESTINATION ${LIB_INSTALL_DIR})
