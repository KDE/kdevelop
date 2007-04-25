
PROJECT(network)

#include_directories(
#        ${COMMONCPP2_INCLUDE_DIRS}
#        ${Boost_INCLUDE_DIRS}
#    )


SET(libfiles
	serverconfig.cpp
	message.cpp
	basicserver.cpp
	basicsession.cpp
	messageimpl.cpp
	helpers.cpp
	teamworkserver.cpp
	teamworksession.cpp
	user.cpp
	teamworkclient.cpp
	forwardsession.cpp
	logger.cpp
	)

SET(testfiles main.cpp )
#this is needed until somebody finds a way to not produce a
#13 Meg static lib in debug build.
SET(CMAKE_BUILD_TYPE release)

#add definitions, compiler switches, etc.

#ADD_DEFINITIONS(-Wall -g -ftemplate-depth-50 -D_REENTRANT -fexceptions -fno-inline )
#ADD_DEFINITIONS(-Wall -g -ftemplate-depth-50 -D_REENTRANT -fexceptions -finline )
#ADD_DEFINITIONS(-Wall -O2 -ftemplate-depth-50 -D_REENTRANT -fexceptions -finline )
ADD_DEFINITIONS(-Wall -ftemplate-depth-50 -D_REENTRANT -fexceptions -finline)
#list all source files here

###### First Target ##########

ADD_LIBRARY( network STATIC ${libfiles} )
TARGET_LINK_LIBRARIES( network ${COMMONCPP2_LIBRARIES} boost_serialization boost_thread)
SET_TARGET_PROPERTIES( network PROPERTIES VERSION 1.0.0 SOVERSION 1)
#INSTALL(TARGETS network DESTINATION ${LIB_INSTALL_DIR})

###### Next Target ##########

ADD_EXECUTABLE( kdevteamwork_server ${testfiles} )
TARGET_LINK_LIBRARIES( kdevteamwork_server boost_thread dl network ${COMMONCPP2_LIBRARIES} )
INSTALL(TARGETS kdevteamwork_server DESTINATION ${BIN_INSTALL_DIR})

#ADD_DEPENDENCIES( kdevteamwork_server network )
