################################################################################################
# This library needs to build without KDE4, please don't add KDE4 specific cmake stuff in here #
################################################################################################

PROJECT(network)

include_directories(
        ${COMMONCPP2_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
    )

SET(libfiles
	serverconfig.cpp
	message.cpp
	basicserver.cpp
	basicsession.cpp
	messageimpl.cpp
	helpers.cpp
	teamworkserver.cpp
	multisession.cpp
	user.cpp
	teamworkclient.cpp
	forwardsession.cpp
	logger.cpp
	sessioninterface.cpp
	dynamicmessagedispatcher.cpp
	)

SET(testfiles main.cpp )

#add definitions, compiler switches, etc.

#ADD_DEFINITIONS(-Wall -ftemplate-depth-50 -D_REENTRANT -fexceptions -finline)
#ADD_DEFINITIONS(-Wall -ftemplate-depth-50 -D_REENTRANT -fexceptions -fno-inline)
ADD_DEFINITIONS(-Wall -ftemplate-depth-50 -D_REENTRANT -fexceptions)
#list all source files here

###### First Target ##########
ADD_LIBRARY( network STATIC ${libfiles} )
TARGET_LINK_LIBRARIES( network ${COMMONCPP2_LIBRARIES} ${Boost_LIBRARIES})
SET_TARGET_PROPERTIES( network PROPERTIES VERSION 1.0.0 SOVERSION 1)
#INSTALL(TARGETS network DESTINATION ${LIB_INSTALL_DIR})

###### Next Target ##########

ADD_EXECUTABLE( kdevteamwork_server ${testfiles} )
TARGET_LINK_LIBRARIES( kdevteamwork_server ${Boost_LIBRARIES} dl network ${COMMONCPP2_LIBRARIES} ${ZLIB_LIBRARIES} )

INSTALL(TARGETS kdevteamwork_server DESTINATION ${BIN_INSTALL_DIR})

#ADD_DEPENDENCIES( kdevteamwork_server network )
