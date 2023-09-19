add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

add_subdirectory(examples)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

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
    viewbarcontainer.cpp
    sublimedefs.cpp
    aggregatemodel.cpp
    holdupdates.cpp

    idealcontroller.cpp
    ideallayout.cpp
    idealtoolbutton.cpp
    idealdockwidget.cpp
    idealbuttonbarwidget.cpp

    message.cpp
    messagewidget.cpp
)
declare_qt_logging_category(sublime_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "sublime"
)
kdevplatform_add_library(KDevPlatformSublime SOURCES ${sublime_LIB_SRCS})
target_link_libraries(KDevPlatformSublime
PUBLIC
    KF6::Parts
PRIVATE
    KF6::KIOWidgets
    KF6::I18n
    KF6::TextWidgets
)

install(FILES
    area.h
    areaindex.h
    areawalkers.h
    container.h
    controller.h
    document.h
    mainwindow.h
    mainwindowoperator.h
    message.h
    urldocument.h
    sublimedefs.h
    tooldocument.h
    view.h
    viewbarcontainer.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/sublime COMPONENT Devel)
