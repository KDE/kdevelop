
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util
    ${CMAKE_SOURCE_DIR}/lib/widgets
    )


########### next target ###############

set(profileengine_LIB_SRCS
    profileengine.cpp
    profile.cpp
)

kde4_automoc(${profileengine_LIB_SRCS})
kde4_add_library(profileengine SHARED ${profileengine_LIB_SRCS})
target_link_libraries(profileengine ${KDE4_KDECORE_LIBS} kio)
set_target_properties(profileengine PROPERTIES VERSION 4.0.0 SOVERSION 4)
install_targets(/lib profileengine)


########### install files ###############

install_files(/include/kdevelop/shell/profileengine FILES profileengine.h profile.h)


