kde4_header()

include_directories(${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-fPIC)

########### next target ###############

set(kdevcpprpp_STAT_SRCS
    preprocessor.cpp
)

kde4_automoc(${kdevcpprpp_STAT_SRCS})
kde4_add_library(kdevcpprpp STATIC ${kdevcpprpp_STAT_SRCS})


########### install files ###############


kde4_footer()
