function(add_debuggable_executable target)
    cmake_parse_arguments(add_debuggable_executable "" "" "SRCS" ${ARGN})
    add_executable(${target} ${add_debuggable_executable_SRCS})
    # force debug symbols for our debuggees, disable optimizations
    if (WIN32)
        set(_flags "/0d")
    else()
        set(_flags "-O0")
    endif()
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} ${_flags}")
endfunction()

configure_file(qtprintersconfig.h.in qtprintersconfig.h)

add_debuggable_executable(qstring SRCS qstring.cpp)
target_link_libraries(qstring ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qbytearray SRCS qbytearray.cpp)
target_link_libraries(qbytearray ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qlistcontainer SRCS qlistcontainer.cpp)
target_link_libraries(qlistcontainer ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qmapint SRCS qmapint.cpp)
target_link_libraries(qmapint ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qmapstring SRCS qmapstring.cpp)
target_link_libraries(qmapstring ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qmapstringbool SRCS qmapstringbool.cpp)
target_link_libraries(qmapstringbool ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qdate SRCS qdate.cpp)
target_link_libraries(qdate ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qtime SRCS qtime.cpp)
target_link_libraries(qtime ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qdatetime SRCS qdatetime.cpp)
target_link_libraries(qdatetime ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qurl SRCS qurl.cpp)
target_link_libraries(qurl ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qhashint SRCS qhashint.cpp)
target_link_libraries(qhashint ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qhashstring SRCS qhashstring.cpp)
target_link_libraries(qhashstring ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qsetint SRCS qsetint.cpp)
target_link_libraries(qsetint ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qsetstring SRCS qsetstring.cpp)
target_link_libraries(qsetstring ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qchar SRCS qchar.cpp)
target_link_libraries(qchar ${QT_QTCORE_LIBRARY})

add_debuggable_executable(quuid SRCS quuid.cpp)
target_link_libraries(quuid ${QT_QTCORE_LIBRARY})

add_debuggable_executable(qlistpod SRCS qlistpod.cpp)
target_link_libraries(qlistpod ${QT_QTCORE_LIBRARY})

add_debuggable_executable(ktexteditortypes SRCS ktexteditortypes.cpp)
target_link_libraries(ktexteditortypes ${QT_QTCORE_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS})

add_debuggable_executable(kdeveloptypes SRCS kdeveloptypes.cpp)
target_link_libraries(kdeveloptypes ${QT_QTCORE_LIBRARY} ${KDEVPLATFORM_UTIL_LIBRARIES})

kde4_add_unit_test(qtprinters qtprinters.cpp)
target_link_libraries(qtprinters
    ${QT_QTCORE_LIBRARY}
    ${QT_QTTEST_LIBRARY}
)
