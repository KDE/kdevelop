include_directories(${CMAKE_SOURCE_DIR}/src/ideal)

########### next target ###############

set(demo1_SRCS
    demo1.cpp
)

kde4_automoc(demo1 ${demo1_SRCS})
kde4_add_executable(demo1 RUN_UNINSTALLED ${demo1_SRCS})
target_link_libraries(demo1 ${KDE4_KDECORE_LIBS} ideal)

########### next target ###############

set(demo2_SRCS
    demo2.cpp
    demowindow2.cpp
)

kde4_automoc(demo2 ${demo2_SRCS})
kde4_add_executable(demo2 RUN_UNINSTALLED ${demo2_SRCS})
target_link_libraries(demo2 ${KDE4_KDECORE_LIBS} ideal)
