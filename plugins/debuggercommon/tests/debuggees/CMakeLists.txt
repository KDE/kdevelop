function(add_debuggable_executable target)
    cmake_parse_arguments(add_debuggable_executable "" "" "SRCS" ${ARGN})
    add_executable(${target} ${add_debuggable_executable_SRCS})
	ecm_mark_nongui_executable(${target})
    # force debug symbols for our debuggees, disable optimizations
    if (WIN32)
        # Avoid "cl : Command line warning D9025 : overriding '/O2' with '/Od'" warnings
        # Thus set COMPILE_FLAGS_RELEASE instead of COMPILE_FLAGS
        # COMPILE_FLAGS_DEBUG should already have the desired flags
        set_target_properties(${target} PROPERTIES COMPILE_FLAGS_RELEASE "/DEBUG /Od")
    else()
        # explicitly disable sanitizers, in case they got enabled globally
        set_target_properties(${target} PROPERTIES COMPILE_FLAGS "-g -O0 -fno-sanitize=address,undefined")
        set_target_properties(${target} PROPERTIES LINK_FLAGS "-g -O0 -fno-sanitize=address,undefined")
    endif()
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

add_debuggable_executable(debuggee_debugee SRCS debugee.cpp)
add_debuggable_executable(debuggee_debugeeslow SRCS debugeeslow.cpp)
add_debuggable_executable(debuggee_crash SRCS debugeecrash.cpp)
add_debuggable_executable(debuggee_debugeerecursion SRCS debugeerecursion.cpp)
add_debuggable_executable(debuggee_debugeespace SRCS "debugee space.cpp")
add_debuggable_executable(debuggee_debugeemultilocbreakpoint SRCS debugeemultilocbreakpoint.cpp)
add_debuggable_executable(debuggee_debugeemultiplebreakpoint SRCS debugeemultiplebreakpoint.cpp)
add_debuggable_executable(debuggee_debugeeechoenv SRCS debugeeechoenv.cpp)
add_debuggable_executable(debuggee_debugeepath SRCS debugeepath.cpp)

add_debuggable_executable(debuggee_debugeethreads SRCS debugeethreads.cpp)
target_link_libraries(debuggee_debugeethreads Qt::Core)

add_debuggable_executable(debuggee_debugeeqt SRCS debugeeqt.cpp)
target_link_libraries(debuggee_debugeeqt Qt::Core)

add_debuggable_executable(debuggee_debugeeexception SRCS debugeeexception.cpp)
kde_target_enable_exceptions(debuggee_debugeeexception PRIVATE)

add_subdirectory("path with space")

add_debuggable_executable(debuggee_qstring SRCS qstring.cpp)
target_link_libraries(debuggee_qstring Qt::Core)

add_debuggable_executable(debuggee_qbytearray SRCS qbytearray.cpp)
target_link_libraries(debuggee_qbytearray Qt::Core)

add_debuggable_executable(debuggee_qlistcontainer SRCS qlistcontainer.cpp)
target_link_libraries(debuggee_qlistcontainer Qt::Core Qt::Core5Compat)

add_debuggable_executable(debuggee_qmapint SRCS qmapint.cpp)
target_link_libraries(debuggee_qmapint Qt::Core)

add_debuggable_executable(debuggee_qmapstring SRCS qmapstring.cpp)
target_link_libraries(debuggee_qmapstring Qt::Core)

add_debuggable_executable(debuggee_qmapstringbool SRCS qmapstringbool.cpp)
target_link_libraries(debuggee_qmapstringbool Qt::Core)

add_debuggable_executable(debuggee_qdate SRCS qdate.cpp)
target_link_libraries(debuggee_qdate Qt::Core)

add_debuggable_executable(debuggee_qtime SRCS qtime.cpp)
target_link_libraries(debuggee_qtime Qt::Core)

add_debuggable_executable(debuggee_qdatetime SRCS qdatetime.cpp)
target_link_libraries(debuggee_qdatetime Qt::Core)

add_debuggable_executable(debuggee_qurl SRCS qurl.cpp)
target_link_libraries(debuggee_qurl Qt::Core)

add_debuggable_executable(debuggee_qhashint SRCS qhashint.cpp)
target_link_libraries(debuggee_qhashint Qt::Core)

add_debuggable_executable(debuggee_qhashstring SRCS qhashstring.cpp)
target_link_libraries(debuggee_qhashstring Qt::Core)

add_debuggable_executable(debuggee_qmultihashint SRCS qmultihashint.cpp)
target_link_libraries(debuggee_qmultihashint Qt::Core)

add_debuggable_executable(debuggee_qsetint SRCS qsetint.cpp)
target_link_libraries(debuggee_qsetint Qt::Core)

add_debuggable_executable(debuggee_qsetstring SRCS qsetstring.cpp)
target_link_libraries(debuggee_qsetstring Qt::Core)

add_debuggable_executable(debuggee_qchar SRCS qchar.cpp)
target_link_libraries(debuggee_qchar Qt::Core)

add_debuggable_executable(debuggee_quuid SRCS quuid.cpp)
target_link_libraries(debuggee_quuid Qt::Core)

add_debuggable_executable(debuggee_qvariant SRCS qvariant.cpp)
target_link_libraries(debuggee_qvariant Qt::Core)

add_debuggable_executable(debuggee_qlistpod SRCS qlistpod.cpp)
target_link_libraries(debuggee_qlistpod Qt::Core)

add_debuggable_executable(debuggee_ktexteditortypes SRCS ktexteditortypes.cpp)
target_link_libraries(debuggee_ktexteditortypes Qt::Core KF6::TextEditor)

add_debuggable_executable(debuggee_kdeveloptypes SRCS kdeveloptypes.cpp)
target_link_libraries(debuggee_kdeveloptypes Qt::Core KF6::TextEditor KDev::Util)
