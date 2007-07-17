include_directories(${CMAKE_SOURCE_DIR})

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
    urldocument.cpp
    switcher.cpp
    tooldocument.cpp
    view.cpp
    view_p.cpp
    viewwidgetcreator.cpp
    sublimedefs.cpp
)

kde4_add_library(sublime SHARED ${sublime_LIB_SRCS})
target_link_libraries(sublime ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})
set_target_properties(sublime PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS sublime DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES
    area.h
    areaindex.h
    areawalkers.h
    container.h
    controller.h
    document.h
    mainwindow.h
    mainwindowoperator.h
    urldocument.h
    sublimedefs.h
    sublimeexport.h
    switcher.h
    tooldocument.h
    view.h
    viewwidgetcreator.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/sublime )
