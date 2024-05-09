set(p4clientstub_SRCS  main.cpp )

add_executable(p4clientstub ${p4clientstub_SRCS} )

target_link_libraries(p4clientstub
       Qt::Core
)
