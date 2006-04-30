kde4_header()

add_subdirectory(profiles)
add_subdirectory(profileengine) 
add_subdirectory(newui) 

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets 
    ${CMAKE_SOURCE_DIR}/src/newui ${CMAKE_SOURCE_DIR}/src/profileengine/lib
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevshell_LIB_SRCS
    api.cpp
    core.cpp
    documentationpart.cpp
    editorproxy.cpp
    generalinfowidget.cpp
    languageselectwidget.cpp
    mainwindowshare.cpp
    partselectwidget.cpp
    plugincontroller.cpp
    projectmanager.cpp
    projectsession.cpp
    splashscreen.cpp
    statusbar.cpp
    toplevel.cpp
    shellextension.cpp
    simplemainwindow.cpp
    documentcontroller.cpp
)

set(kdevshell_UI
    generalinfowidgetbase.ui
    mimewarningdialog.ui
)

kde4_automoc(${kdevshell_LIB_SRCS})
kde4_add_ui3_files(kdevshell_LIB_SRCS ${kdevshell_UI})
kde4_add_library(kdevshell SHARED ${kdevshell_LIB_SRCS})
target_link_libraries(kdevshell ${KDE4_KDECORE_LIBS} kdeui profileengine kdevinterfaces kdevutil kdevwidgets newui khtml ktexteditor)
set_target_properties(kdevshell PROPERTIES VERSION 4.0.0 SOVERSION 4)
install_targets(/lib kdevshell)


########### next target ###############

set(kdevelop_SRCS
    main.cpp
    kdevideextension.cpp
)

set(kdevelop_UI
    settingswidget.ui
)

kde4_automoc(${kdevelop_SRCS})
kde4_add_ui3_files(kdevelop_SRCS ${kdevelop_UI})
kde4_add_executable(kdevelop ${kdevelop_SRCS})
target_link_libraries(kdevelop ${KDE4_KDECORE_LIBS} kdevshell)
install_targets(/bin kdevelop)


########### next target ###############

set(kdevassistant_SRCS 
    main_assistant.cpp
    kdevassistantextension.cpp
)

kde4_automoc(${kdevassistant_SRCS})
kde4_add_executable(kdevassistant ${kdevassistant_SRCS})
target_link_libraries(kdevassistant ${KDE4_KDECORE_LIBS} kdevshell)
install_targets(/bin kdevassistant)


########### install files ###############

install_files(/include/kdevelop/shell FILES 
    api.h core.h documentationpart.h editorproxy.h
    generalinfowidget.h languageselectwidget.h mainwindowshare.h
    partselectwidget.h plugincontroller.h projectmanager.h 
    projectsession.h shellextension.h splashscreen.h statusbar.h
    toplevel.h generalinfowidgetbase.h mimewarningdialog.h settingswidget.h
    simplemainwindow.h documentcontroller.h shellexport.h)

install_files(${CONFIG_INSTALL_DIR} FILES kdeveloprc kdevassistantrc)
install_files(${DATA_INSTALL_DIR}/kdevelop FILES kdevelopui.rc eventsrc)
install_files(${DATA_INSTALL_DIR}/kdevelop/profiles FILES projectprofiles)
install_files(${DATA_INSTALL_DIR}/kdevassistant FILES kdevassistantui.rc)


kde4_footer()
