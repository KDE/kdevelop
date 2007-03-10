include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
)

set(kdevplatformconfig_LIB_SRCS
    kdevcmmodule.cpp
    kdevconfig.cpp
    kdevconfigskeleton.cpp
)

kde4_automoc(${kdevplatformconfig_LIB_SRCS})

kde4_add_library(kdevplatformconfig SHARED ${kdevplatformconfig_LIB_SRCS})
target_link_libraries(kdevplatformconfig ${KDE4_CORE_LIBS} kdevplatforminterfaces )
set_target_properties(kdevplatformconfig PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformconfig DESTINATION ${LIB_INSTALL_DIR} )
install(FILES
    kdevconfig.h
    kdevconfigskeleton.h
    kdevcmmodule.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/config
)
