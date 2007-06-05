add_subdirectory(profiles)
# add_subdirectory(profileengine)

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}/sublime
    ${KDEVPLATFORM_INCLUDE_DIR}/shell
    ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
    )


########### next target ###############

set(kdevelop_SRCS
    main.cpp
    kdevideextension.cpp
    splashscreen.cpp
)

kde4_automoc(${kdevelop_SRCS})
kde4_add_executable(kdevelop ${kdevelop_SRCS})
target_link_libraries(kdevelop ${KDE4_KDECORE_LIBS} ${KDEVPLATFORM_SHELL_LIBRARY})
install(TARGETS kdevelop DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdeveloprc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES kdevelopui.rc kdevelop.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
