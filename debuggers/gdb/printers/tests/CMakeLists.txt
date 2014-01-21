# force debug symbols for our debugees
set(CMAKE_BUILD_TYPE debugfull)

configure_file(qtprintersconfig.h.in qtprintersconfig.h)

add_executable(qstring qstring.cpp)
target_link_libraries(qstring Qt5::Core)

add_executable(qbytearray qbytearray.cpp)
target_link_libraries(qbytearray Qt5::Core)

add_executable(qlistcontainer qlistcontainer.cpp)
target_link_libraries(qlistcontainer Qt5::Core)

add_executable(qmapint qmapint.cpp)
target_link_libraries(qmapint Qt5::Core)

add_executable(qmapstring qmapstring.cpp)
target_link_libraries(qmapstring Qt5::Core)

add_executable(qmapstringbool qmapstringbool.cpp)
target_link_libraries(qmapstringbool Qt5::Core)

add_executable(qdate qdate.cpp)
target_link_libraries(qdate Qt5::Core)

add_executable(qtime qtime.cpp)
target_link_libraries(qtime Qt5::Core)

add_executable(qdatetime qdatetime.cpp)
target_link_libraries(qdatetime Qt5::Core)

add_executable(qurl qurl.cpp)
target_link_libraries(qurl Qt5::Core)

add_executable(qhashint qhashint.cpp)
target_link_libraries(qhashint Qt5::Core)

add_executable(qhashstring qhashstring.cpp)
target_link_libraries(qhashstring Qt5::Core)

add_executable(qsetint qsetint.cpp)
target_link_libraries(qsetint Qt5::Core)

add_executable(qsetstring qsetstring.cpp)
target_link_libraries(qsetstring Qt5::Core)

add_executable(qchar qchar.cpp)
target_link_libraries(qchar Qt5::Core)

add_executable(quuid quuid.cpp)
target_link_libraries(quuid Qt5::Core)

add_executable(qlistpod qlistpod.cpp)
target_link_libraries(qlistpod Qt5::Core)

add_executable(ktexteditortypes ktexteditortypes.cpp)
target_link_libraries(ktexteditortypes Qt5::Core KF5::TextEditor)

ecm_add_test(qtprinters
LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
)
