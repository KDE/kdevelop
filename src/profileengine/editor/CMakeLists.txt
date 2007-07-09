
include_directories(${KDEVPLATFORM_INCLUDE_DIR} ${KDEVPLATFORM_INCLUDE_DIR}/util
    ${KDEVPLATFORM_INCLUDE_DIR}/widgets
    )


########### next target ###############

set(profileeditor_STAT_SRCS
    profileeditor.cpp
)



set(profileeditor_UI
    profileeditorbase.ui
    addprofilewidget.ui
)

kde4_add_ui3_files(profileeditor_STAT_SRCS ${profileeditor_UI})
kde4_add_library(profileeditor STATIC ${profileeditor_STAT_SRCS})


########### next target ###############

set(kdevprofileeditor_SRCS
    main.cpp
)


kde4_add_executable(kdevprofileeditor ${kdevprofileeditor_SRCS})
target_link_libraries(kdevprofileeditor ${KDE4_KDECORE_LIBS} kdevplatform profileeditor ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kdevprofileeditor DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############


