

set(headergen_SRCS
    main.cpp
)

kde4_add_test_executable(headergen ${headergen_SRCS})
target_link_libraries(headergen ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS} kdev4cpprpp kdev4cppparser)
