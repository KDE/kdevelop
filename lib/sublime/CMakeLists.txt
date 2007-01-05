include_directories(${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(examples)
add_subdirectory(test)

########### next target ###############

set(sublime_LIB_SRCS
    area.cpp
    areaindex.cpp
    container.cpp
    controller.cpp
    document.cpp
    mainwindow.cpp
    mainwindow_p.cpp
    mainwindowoperator.cpp
    partdocument.cpp
    switcher.cpp
    tooldocument.cpp
    view.cpp
    view_p.cpp
    viewcreator.cpp
    viewwidgetcreator.cpp
)

# set(sublime_UI
# )

kde4_automoc(${sublime_LIB_SRCS})
# kde4_add_ui_files(sublime_LIB_SRCS ${sublime_UI})
kde4_add_library(sublime SHARED ${sublime_LIB_SRCS})
target_link_libraries(sublime ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})
set_target_properties(sublime PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS sublime DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES
    area.h
    areaindex.h
    container.h
    controller.h
    document.h
    mainwindow.h
    mainwindowoperator.h
    partdocument.h
    sublimedefs.h
    switcher.h
    tooldocument.h
    view.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/sublime )
