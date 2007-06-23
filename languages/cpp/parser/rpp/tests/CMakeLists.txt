include_directories( ${CMAKE_SOURCE_DIR}/languages/cpp/parser ${CMAKE_CURRENT_SOURCE_DIR}/..  )


########### next target ###############

set(pp_SRCS
    main.cpp
)

kde4_add_test_executable(pp ${pp_SRCS})
target_link_libraries(pp ${KDE4_KDECORE_LIBS} kdevcpprpp)

