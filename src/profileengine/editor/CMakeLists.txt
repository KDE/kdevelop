
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util
    ${CMAKE_SOURCE_DIR}/lib/widgets
    )


########### next target ###############

set(profileeditor_STAT_SRCS
    profileeditor.cpp
)

kde4_automoc(${profileeditor_STAT_SRCS})

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

kde4_automoc(${kdevprofileeditor_SRCS})
kde4_add_executable(kdevprofileeditor ${kdevprofileeditor_SRCS})
target_link_libraries(kdevprofileeditor ${KDE4_KDECORE_LIBS} kdevinterfaces profileeditor ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kdevprofileeditor DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############


