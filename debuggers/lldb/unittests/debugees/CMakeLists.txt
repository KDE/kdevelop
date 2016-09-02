add_debuggable_executable(lldb_debugee SRCS debugee.cpp)
add_debuggable_executable(lldb_debugeeslow SRCS debugeeslow.cpp)
add_debuggable_executable(lldb_debugeecrash SRCS debugeecrash.cpp)
add_debuggable_executable(lldb_debugeerecursion SRCS debugeerecursion.cpp)
add_debuggable_executable(lldb_debugeespace SRCS "debugee space.cpp")
add_debuggable_executable(lldb_debugeemultilocbreakpoint SRCS debugeemultilocbreakpoint.cpp)
add_debuggable_executable(lldb_debugeemultiplebreakpoint SRCS debugeemultiplebreakpoint.cpp)
add_debuggable_executable(lldb_debugeeechoenv SRCS debugeeechoenv.cpp)
add_debuggable_executable(lldb_debugeepath SRCS debugeepath.cpp)

add_debuggable_executable(lldb_debugeethreads SRCS debugeethreads.cpp)
target_link_libraries(lldb_debugeethreads Qt5::Core)

add_debuggable_executable(lldb_debugeeqt SRCS debugeeqt.cpp)
target_link_libraries(lldb_debugeeqt Qt5::Core)

add_debuggable_executable(lldb_debugeeexception SRCS debugeeexception.cpp)
kde_target_enable_exceptions(lldb_debugeeexception PRIVATE)

if (HAVE_PATH_WITH_SPACES_TEST)
    add_subdirectory("path with space")
endif()


include_directories(KF5::TextEditor)

add_debuggable_executable(lldb_qstring SRCS qstring.cpp)
target_link_libraries(lldb_qstring Qt5::Core)

add_debuggable_executable(lldb_qbytearray SRCS qbytearray.cpp)
target_link_libraries(lldb_qbytearray Qt5::Core)

add_debuggable_executable(lldb_qlistcontainer SRCS qlistcontainer.cpp)
target_link_libraries(lldb_qlistcontainer Qt5::Core)

add_debuggable_executable(lldb_qmapint SRCS qmapint.cpp)
target_link_libraries(lldb_qmapint Qt5::Core)

add_debuggable_executable(lldb_qmapstring SRCS qmapstring.cpp)
target_link_libraries(lldb_qmapstring Qt5::Core)

add_debuggable_executable(lldb_qmapstringbool SRCS qmapstringbool.cpp)
target_link_libraries(lldb_qmapstringbool Qt5::Core)

add_debuggable_executable(lldb_qdate SRCS qdate.cpp)
target_link_libraries(lldb_qdate Qt5::Core)

add_debuggable_executable(lldb_qtime SRCS qtime.cpp)
target_link_libraries(lldb_qtime Qt5::Core)

add_debuggable_executable(lldb_qdatetime SRCS qdatetime.cpp)
target_link_libraries(lldb_qdatetime Qt5::Core)

add_debuggable_executable(lldb_qurl SRCS qurl.cpp)
target_link_libraries(lldb_qurl Qt5::Core)

add_debuggable_executable(lldb_qhashint SRCS qhashint.cpp)
target_link_libraries(lldb_qhashint Qt5::Core)

add_debuggable_executable(lldb_qhashstring SRCS qhashstring.cpp)
target_link_libraries(lldb_qhashstring Qt5::Core)

add_debuggable_executable(lldb_qsetint SRCS qsetint.cpp)
target_link_libraries(lldb_qsetint Qt5::Core)

add_debuggable_executable(lldb_qsetstring SRCS qsetstring.cpp)
target_link_libraries(lldb_qsetstring Qt5::Core)

add_debuggable_executable(lldb_qchar SRCS qchar.cpp)
target_link_libraries(lldb_qchar Qt5::Core)

add_debuggable_executable(lldb_quuid SRCS quuid.cpp)
target_link_libraries(lldb_quuid Qt5::Core)

add_debuggable_executable(lldb_qlistpod SRCS qlistpod.cpp)
target_link_libraries(lldb_qlistpod Qt5::Core)

add_debuggable_executable(lldb_ktexteditortypes SRCS ktexteditortypes.cpp)
target_link_libraries(lldb_ktexteditortypes Qt5::Core KF5::TextEditor KDev::Util)

add_debuggable_executable(lldb_kdeveloptypes SRCS kdeveloptypes.cpp)
target_link_libraries(lldb_kdeveloptypes Qt5::Core KF5::TextEditor KDev::Util)
