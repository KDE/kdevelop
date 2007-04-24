
PROJECT(network)

include_directories( ${ccgnu2_includes}/cc++2 )

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(libfiles serverconfig.cpp message.cpp basicserver.cpp basicsession.cpp messageimpl.cpp helpers.cpp teamworkserver.cpp teamworksession.cpp user.cpp teamworkclient.cpp forwardsession.cpp logger.cpp)
SET(testfiles main.cpp )

LINK_LIBRARIES(ccgnu2 dl boost_serialization)

#add definitions, compiler switches, etc.

#ADD_DEFINITIONS(-Wall -g -ftemplate-depth-50 -D_REENTRANT -fexceptions -fno-inline )
#ADD_DEFINITIONS(-Wall -g -ftemplate-depth-50 -D_REENTRANT -fexceptions -finline )
ADD_DEFINITIONS(-Wall -O2 -ftemplate-depth-50 -D_REENTRANT -fexceptions -finline )

#list all source files here

ADD_LIBRARY( network STATIC ${libfiles} )
ADD_EXECUTABLE( kdevteamwork_server ${testfiles} )

TARGET_LINK_LIBRARIES( kdevteamwork_server boost_thread network ccgnu2 )
ADD_DEPENDENCIES( kdevteamwork_server network ) 
