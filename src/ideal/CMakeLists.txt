########### next target ###############
set(ideal_LIB_SRCS
    button.cpp
    buttonbar.cpp
    buttoncontainer.cpp
    mainwindow.cpp
    toolview.cpp
    area.cpp
    settings.cpp
    toolviewwidget.cpp
)

kde4_automoc(${ideal_LIB_SRCS})
kde4_add_library(ideal SHARED ${ideal_LIB_SRCS})
target_link_libraries(ideal ${KDE4_KDECORE_LIBS} kparts)
set_target_properties(ideal PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS ideal DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES area.h button.h buttonbar.h mainwindow.h toolview.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/ideal)

add_subdirectory(test)
