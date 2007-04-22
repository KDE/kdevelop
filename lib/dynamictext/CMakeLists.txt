PROJECT( dynamictext )

include_directories( ${ccgnu2_includes}/cc++2 ${CMAKE_SOURCE_DIR}/lib/network ${CMAKE_SOURCE_DIR})

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall -g -fno-inline -fexceptions)
#ADD_DEFINITIONS(-Wall -os -finline -fexceptions)
#ADD_DEFINITIONS(-Wall -o3 -g -fexceptions)

#list all source files here

LINK_LIBRARIES(ccgnu2 dl boost_serialization)

ADD_LIBRARY( dynamictext dynamictext.cpp vectortimestamp.cpp offsetutils.cpp sumtree.cpp )

ADD_EXECUTABLE( testexec main.cpp)
TARGET_LINK_LIBRARIES( testexec dynamictext boost_thread )

#ADD_DEPENDENCIES( testexec, dynamictext )
 
SET(CMAKE_CXX_FLAGS -fPIC)
SET(CMAKE_C_FLAGS -fPIC)
