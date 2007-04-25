PROJECT( dynamictext )
# include_directories(
#         ${COMMONCPP2_INCLUDE_DIRS}
#         ${Boost_INCLUDE_DIRS}
#     )

#if you don't want the full compiler output, remove the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

SET( dynamictext_SRCS
	dynamictext.cpp
	vectortimestamp.cpp
	offsetutils.cpp
	sumtree.cpp
)

#this is needed until somebody finds a way to not produce a
#13 Meg static lib in debug build.
SET(CMAKE_BUILD_TYPE release)

#add definitions, compiler switches, etc.
#ADD_DEFINITIONS(-Wall -g -fno-inline -fexceptions)
#ADD_DEFINITIONS(-Wall -os -finline -fexceptions)
#ADD_DEFINITIONS(-Wall -o3 -g -fexceptions)

ADD_DEFINITIONS(-Wall -fexceptions)

#list all source files here

######## next target ########

ADD_LIBRARY( dynamictext STATIC ${dynamictext_SRCS} )
TARGET_LINK_LIBRARIES( dynamictext ${COMMONCPP2_LIBRARIES} boost_serialization boost_thread)
set_target_properties( dynamictext PROPERTIES VERSION 1.0.0 SOVERSION 1)
#install(TARGETS dynamictext DESTINATION ${LIB_INSTALL_DIR})

ADD_EXECUTABLE( testexec main.cpp)
TARGET_LINK_LIBRARIES( testexec dynamictext boost_thread dl )

#ADD_DEPENDENCIES( testexec, dynamictext )

#SET(CMAKE_CXX_FLAGS -fPIC)
#SET(CMAKE_C_FLAGS -fPIC)
