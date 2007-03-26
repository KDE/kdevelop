if(SUBVERSIONFOUND)
    add_subdirectory(subversion)
endif(SUBVERSIONFOUND)

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/lib/interfaces
)

set(kdevplatformvcs_LIB_SRCS
    interfaces/iversioncontrol.cpp
)

kde4_automoc(${kdevplatformvcs_LIB_SRCS})

kde4_add_library(kdevplatformvcs SHARED ${kdevplatformvcs_LIB_SRCS})
target_link_libraries(kdevplatformvcs ${KDE4_KDECORE_LIBS}  )
set_target_properties(kdevplatformvcs PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformvcs DESTINATION ${LIB_INSTALL_DIR} )

install(FILES
    interfaces/iversioncontrol.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/vcs/interfaces
)
