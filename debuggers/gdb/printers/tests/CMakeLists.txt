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

include_directories(KF5::TextEditor)

add_debuggable_executable(qstring SRCS qstring.cpp)
target_link_libraries(qstring Qt5::Core)

add_debuggable_executable(qbytearray SRCS qbytearray.cpp)
target_link_libraries(qbytearray Qt5::Core)

add_debuggable_executable(qlistcontainer SRCS qlistcontainer.cpp)
target_link_libraries(qlistcontainer Qt5::Core)

add_debuggable_executable(qmapint SRCS qmapint.cpp)
target_link_libraries(qmapint Qt5::Core)

add_debuggable_executable(qmapstring SRCS qmapstring.cpp)
target_link_libraries(qmapstring Qt5::Core)

add_debuggable_executable(qmapstringbool SRCS qmapstringbool.cpp)
target_link_libraries(qmapstringbool Qt5::Core)

add_debuggable_executable(qdate SRCS qdate.cpp)
target_link_libraries(qdate Qt5::Core)

add_debuggable_executable(qtime SRCS qtime.cpp)
target_link_libraries(qtime Qt5::Core)

add_debuggable_executable(qdatetime SRCS qdatetime.cpp)
target_link_libraries(qdatetime Qt5::Core)

add_debuggable_executable(qurl SRCS qurl.cpp)
target_link_libraries(qurl Qt5::Core)

add_debuggable_executable(qhashint SRCS qhashint.cpp)
target_link_libraries(qhashint Qt5::Core)

add_debuggable_executable(qhashstring SRCS qhashstring.cpp)
target_link_libraries(qhashstring Qt5::Core)

add_debuggable_executable(qsetint SRCS qsetint.cpp)
target_link_libraries(qsetint Qt5::Core)

add_debuggable_executable(qsetstring SRCS qsetstring.cpp)
target_link_libraries(qsetstring Qt5::Core)

add_debuggable_executable(qchar SRCS qchar.cpp)
target_link_libraries(qchar Qt5::Core)

add_debuggable_executable(quuid SRCS quuid.cpp)
target_link_libraries(quuid Qt5::Core)

add_debuggable_executable(qlistpod SRCS qlistpod.cpp)
target_link_libraries(qlistpod Qt5::Core)

add_debuggable_executable(ktexteditortypes SRCS ktexteditortypes.cpp)
target_link_libraries(ktexteditortypes Qt5::Core KF5::TextEditor)

add_debuggable_executable(kdeveloptypes SRCS kdeveloptypes.cpp)
target_link_libraries(kdeveloptypes Qt5::Core KF5::TextEditor ${KDEVPLATFORM_UTIL_LIBRARIES})

ecm_add_test(qtprinters
LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
)
