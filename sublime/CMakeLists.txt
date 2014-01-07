add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9504 )
add_subdirectory(examples)
add_subdirectory(tests)

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
    tooldocument.cpp
    view.cpp
    sublimedefs.cpp
    aggregatemodel.cpp
    holdupdates.cpp

    idealcontroller.cpp
    ideallayout.cpp
    idealtoolbutton.cpp
    idealdockwidget.cpp
    idealbuttonbarwidget.cpp
)

add_library(sublime SHARED ${sublime_LIB_SRCS})
target_link_libraries(sublime
        LINK_PRIVATE ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
        LINK_PUBLIC ${KDE4_KPARTS_LIBS})
set_target_properties(sublime PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS sublime EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


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
    tooldocument.h
    view.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/sublime COMPONENT Devel)
