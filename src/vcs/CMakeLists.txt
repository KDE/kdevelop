include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/interfaces
)

set(kdevplatformvcs_LIB_SRCS
    iversioncontrol.cpp
)

kde4_automoc(${kdevplatformvcs_LIB_SRCS})

kde4_add_library(kdevplatformvcs SHARED ${kdevplatformvcs_LIB_SRCS})
target_link_libraries(kdevplatformvcs ${KDE4_KDECORE_LIBS}  )
set_target_properties(kdevplatformvcs PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformvcs DESTINATION ${LIB_INSTALL_DIR} )

install(FILES
    vcsexport.h
    iversioncontrol.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs
)

