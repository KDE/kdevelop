include_directories(${CMAKE_SOURCE_DIR}/src/ideal)

########### next target ###############

set(demo1_SRCS
    demo1.cpp
)

kde4_automoc(${demo1_SRCS})
kde4_add_executable(demo1 RUN_UNINSTALLED ${demo1_SRCS})
target_link_libraries(demo1 ${KDE4_KDECORE_LIBS} ideal)
