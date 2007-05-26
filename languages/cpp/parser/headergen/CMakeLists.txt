
include_directories( 
	${CMAKE_SOURCE_DIR}/lib/language/duchain 
	${CMAKE_SOURCE_DIR}/lib/language
	${CMAKE_SOURCE_DIR}/languages/cpp/parser )

set(headergen_SRCS
    main.cpp
)

kde4_add_test(headergen ${headergen_SRCS})
target_link_libraries(headergen ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS} kdevcpprpp kdevcppparser)
