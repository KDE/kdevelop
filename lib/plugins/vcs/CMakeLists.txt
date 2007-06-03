macro_optional_add_subdirectory(cvs)

find_package(SubversionLibrary)

if(SUBVERSION_FOUND)
    macro_optional_add_subdirectory(subversion)
endif(SUBVERSION_FOUND)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}
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
install(FILES
    vcsexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/vcs
)

