kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/src/profileengine/lib ${KDE4_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(newui_LIB_SRCS
    dmainwindow.cpp
    ddockwindow.cpp
    docksplitter.cpp
    dtabwidget.cpp
)

kde4_automoc(${newui_LIB_SRCS})
kde4_add_library(newui SHARED ${newui_LIB_SRCS})
target_link_libraries(newui ${KDE4_KDECORE_LIBS} kparts)
set_target_properties(newui PROPERTIES VERSION 4.2.0 SOVERSION 4)
install_targets(/lib newui)


########### install files ###############

install_files(/include/kdevelop/shell FILES dmainwindow.h ddockwindow.h docksplitter.h dtabwidget.h)


kde4_footer()
