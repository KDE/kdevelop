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
target_link_libraries(debuggee_debugeethreads Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_debugeeqt SRCS debugeeqt.cpp)
target_link_libraries(debuggee_debugeeqt Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_debugeeexception SRCS debugeeexception.cpp)
kde_target_enable_exceptions(debuggee_debugeeexception PRIVATE)

add_subdirectory("path with space")

add_debuggable_executable(debuggee_qstring SRCS qstring.cpp)
target_link_libraries(debuggee_qstring Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qbytearray SRCS qbytearray.cpp)
target_link_libraries(debuggee_qbytearray Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qlistcontainer SRCS qlistcontainer.cpp)
target_compile_options(debuggee_qlistcontainer PRIVATE -UQT_DISABLE_DEPRECATED_BEFORE)
target_link_libraries(debuggee_qlistcontainer Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qmapint SRCS qmapint.cpp)
target_link_libraries(debuggee_qmapint Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qmapstring SRCS qmapstring.cpp)
target_link_libraries(debuggee_qmapstring Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qmapstringbool SRCS qmapstringbool.cpp)
target_link_libraries(debuggee_qmapstringbool Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qdate SRCS qdate.cpp)
target_link_libraries(debuggee_qdate Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qtime SRCS qtime.cpp)
target_link_libraries(debuggee_qtime Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qdatetime SRCS qdatetime.cpp)
target_link_libraries(debuggee_qdatetime Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qurl SRCS qurl.cpp)
target_link_libraries(debuggee_qurl Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qhashint SRCS qhashint.cpp)
target_link_libraries(debuggee_qhashint Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qhashstring SRCS qhashstring.cpp)
target_link_libraries(debuggee_qhashstring Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qsetint SRCS qsetint.cpp)
target_link_libraries(debuggee_qsetint Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qsetstring SRCS qsetstring.cpp)
target_link_libraries(debuggee_qsetstring Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qchar SRCS qchar.cpp)
target_link_libraries(debuggee_qchar Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_quuid SRCS quuid.cpp)
target_link_libraries(debuggee_quuid Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qvariant SRCS qvariant.cpp)
target_link_libraries(debuggee_qvariant Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_qlistpod SRCS qlistpod.cpp)
target_link_libraries(debuggee_qlistpod Qt${QT_MAJOR_VERSION}::Core)

add_debuggable_executable(debuggee_ktexteditortypes SRCS ktexteditortypes.cpp)
target_link_libraries(debuggee_ktexteditortypes Qt${QT_MAJOR_VERSION}::Core KF${KF_MAJOR_VERSION}::TextEditor)

add_debuggable_executable(debuggee_kdeveloptypes SRCS kdeveloptypes.cpp)
target_link_libraries(debuggee_kdeveloptypes Qt${QT_MAJOR_VERSION}::Core KF${KF_MAJOR_VERSION}::TextEditor KDev::Util)
