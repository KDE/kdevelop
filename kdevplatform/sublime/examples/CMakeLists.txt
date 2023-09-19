########### next target ###############

set(example1_SRCS
    example1main.cpp
    example1.cpp
)

add_executable(example1 ${example1_SRCS})
target_link_libraries(example1 KF6::Parts KDev::Sublime)

########### next target ###############

set(example2_SRCS
    example2main.cpp
    example2.cpp
)

add_executable(example2 ${example2_SRCS})
target_link_libraries(example2 KF6::Parts KDev::Sublime)
