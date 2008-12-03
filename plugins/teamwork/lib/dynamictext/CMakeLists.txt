################################################################################################
# This library needs to build without KDE4, please don't add KDE4 specific cmake stuff in here #
################################################################################################

PROJECT( dynamictext )

#if you don't want the full compiler output, remove the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

SET( dynamictext_SRCS
    dynamictext.cpp
    vectortimestamp.cpp
    offsetutils.cpp
    sumtree.cpp
)

#add definitions, compiler switches, etc.
#ADD_DEFINITIONS(-Wall -g -fno-inline -fexceptions)
#ADD_DEFINITIONS(-Wall -os -finline -fexceptions)
#ADD_DEFINITIONS(-Wall -o3 -g -fexceptions)

ADD_DEFINITIONS(-Wall ${KDE4_ENABLE_EXCEPTIONS})

#list all source files here

######## next target ########

# This library only exists as a convenience library for the teamwork plugin, thus it is not SOVERSION'ed
ADD_LIBRARY( kdevteamwork_dynamictext SHARED ${dynamictext_SRCS} )
TARGET_LINK_LIBRARIES( kdevteamwork_dynamictext ${COMMONCPP2_LIBRARIES} ${Boost_THREAD_LIBRARY} ${Boost_SERIALIZATION_LIBRARY} ${QT_QTCORE_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS kdevteamwork_dynamictext ${INSTALL_TARGETS_DEFAULT_ARGS})

ADD_EXECUTABLE( testexec main.cpp)
TARGET_LINK_LIBRARIES( testexec kdevteamwork_dynamictext ${Boost_THREAD_LIBRARY} ${Boost_SERIALIZATION_LIBRARY} dl ${ZLIB_LIBRARIES} )

#ADD_DEPENDENCIES( testexec, dynamictext )

