########### next target ###############

set(example1_SRCS
    example1main.cpp
    example1.cpp
)

add_executable(example1 ${example1_SRCS})
target_link_libraries(example1 ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime)

########### next target ###############

set(example2_SRCS
    example2main.cpp
    example2.cpp
)

add_executable(example2 ${example2_SRCS})
target_link_libraries(example2 ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} sublime)
