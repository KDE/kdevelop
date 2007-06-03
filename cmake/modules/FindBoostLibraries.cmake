# This tries to find the boost libraries needed for the teamwork plugin
# it uses the FindBoost cmake script, but additionally to that it puts
# the right library names into the extra variable. This is needed for
# systems like Debian were a broken package is shipped with both -st and -mt
# libs.
#
# Sets Boost_LIBRARIES to the libraries boost_serialization and boost_thread

if(Boost_LIBRARIES)
    set(Boost_FIND_QUIETLY TRUE)
endif(Boost_LIBRARIES)

find_package(Boost)

if(NOT Boost_FOUND)
    message(STATUS "Couldn't find Boost, will not try to find the libraries")
    set(Boost_LIBS_FOUND FALSE)
else(NOT Boost_FOUND)
    find_library(Boost_SERIALIZATION NAMES boost_serialization boost_serialization-mt
        PATHS
        Boost_LIBRARY_DIRS
    )
    
    find_library(Boost_THREAD NAMES boost_thread boost_thread-mt
        PATHS
        Boost_LIBRARY_DIRS
    )

    if(NOT Boost_THREAD OR NOT Boost_SERIALIZATION)
        message(STATUS "Couldn't find boost libraries")
        set(Boost_LIBS_FOUND FALSE)
    else(NOT Boost_THREAD OR NOT Boost_SERIALIZATION)
        set(Boost_LIBRARIES ${Boost_SERIALIZATION} ${Boost_THREAD})
	set(Boost_LIBRARIES ${Boost_LIBRARIES} CACHE STRING "List of Boost libraries for Kdevelop")
        set(Boost_LIBS_FOUND TRUE)
        if(NOT Boost_FIND_QUIETLY)
            message(STATUS "Found Boost libraries: ${Boost_LIBRARIES}")
        endif(NOT Boost_FIND_QUIETLY)
    endif(NOT Boost_THREAD OR NOT Boost_SERIALIZATION)
    
    MARK_AS_ADVANCED(
       Boost_SERIALIZATION
       Boost_THREAD
       Boost_LIBRARIES
       )
endif(NOT Boost_FOUND)

