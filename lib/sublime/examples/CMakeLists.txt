include_directories(${CMAKE_SOURCE_DIR}/lib)

########### next target ###############

set(example1_SRCS
    example1main.cpp
    example1.cpp
)

kde4_automoc(${example1_SRCS})
kde4_add_executable(example1 ${example1_SRCS})
target_link_libraries(example1 ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime)
