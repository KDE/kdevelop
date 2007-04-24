
PROJECT(network)


SET(libfiles serverconfig.cpp message.cpp basicserver.cpp basicsession.cpp messageimpl.cpp helpers.cpp teamworkserver.cpp teamworksession.cpp user.cpp teamworkclient.cpp forwardsession.cpp logger.cpp)
SET(testfiles main.cpp )

LINK_LIBRARIES(${COMMONCPP2_LIBRARY} dl boost_serialization)

#add definitions, compiler switches, etc.

#ADD_DEFINITIONS(-Wall -g -ftemplate-depth-50 -D_REENTRANT -fexceptions -fno-inline )
#ADD_DEFINITIONS(-Wall -g -ftemplate-depth-50 -D_REENTRANT -fexceptions -finline )
ADD_DEFINITIONS(-Wall -O2 -ftemplate-depth-50 -D_REENTRANT -fexceptions -finline )

#list all source files here

ADD_LIBRARY( network STATIC ${libfiles} )
ADD_EXECUTABLE( kdevteamwork_server ${testfiles} )

TARGET_LINK_LIBRARIES( kdevteamwork_server boost_thread network ${COMMONCPP2_LIBRARY} )
ADD_DEPENDENCIES( kdevteamwork_server network ) 
