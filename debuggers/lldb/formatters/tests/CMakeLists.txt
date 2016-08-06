configure_file(lldbformatterconfig.h.in lldbformatterconfig.h)

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

# Unit tests for printers
ecm_add_test(test_lldbformatters.cpp
    LINK_LIBRARIES
        Qt5::Core
        Qt5::Test
)
