set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


set(headergen_SRCS
    main.cpp
)

kde4_add_executable(headergen TEST ${headergen_SRCS})
target_link_libraries(headergen ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS} kdev4cpprpp kdev4cppparser)
