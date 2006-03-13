kde4_header()

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util 
    ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/src/profileengine/lib ${KDE4_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(d_LIB_SRCS
    dmainwindow.cpp
    buttonbar.cpp
    button.cpp
    ddockwindow.cpp
    docksplitter.cpp
    dtabwidget.cpp
)

kde4_automoc(${d_LIB_SRCS})
kde4_add_library(d SHARED ${d_LIB_SRCS})
target_link_libraries(d ${QT_AND_KDECORE_LIBS} kparts)
set_target_properties(d PROPERTIES VERSION 4.2.0 SOVERSION 4)
install_targets(/lib d)


########### install files ###############

install_files(/include/kdevelop/shell FILES dmainwindow.h ddockwindow.h buttonbar.h button.h comdefs.h docksplitter.h dtabwidget.h)


kde4_footer()
