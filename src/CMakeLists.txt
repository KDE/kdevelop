add_subdirectory(profiles)
# add_subdirectory(profileengine)

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/shell
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    )


########### next target ###############

set(kdevelop_SRCS
    main.cpp
    kdevideextension.cpp
    splashscreen.cpp
)

kde4_automoc(${kdevelop_SRCS})
kde4_add_executable(kdevelop ${kdevelop_SRCS})
target_link_libraries(kdevelop ${KDE4_KDECORE_LIBS} kdevplatformshell)
install(TARGETS kdevelop DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdeveloprc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES kdevelopui.rc kdevelop.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
