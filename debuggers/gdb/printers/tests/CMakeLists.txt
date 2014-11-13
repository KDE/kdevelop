# force debug symbols for our debugees, disable optimizations
if (DEFINED CMAKE_CXX_FLAGS_DEBUGFULL)
    set(CMAKE_BUILD_TYPE "debugfull")
else()
    # FIXME: this will probably lead to failures in the unit tests
    message(WARNING "The GDB unit tests may fail when no debugfull configuration is available.")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

configure_file(qtprintersconfig.h.in qtprintersconfig.h)

add_executable(qstring qstring.cpp)
target_link_libraries(qstring ${QT_QTCORE_LIBRARY})

add_executable(qbytearray qbytearray.cpp)
target_link_libraries(qbytearray ${QT_QTCORE_LIBRARY})

add_executable(qlistcontainer qlistcontainer.cpp)
target_link_libraries(qlistcontainer ${QT_QTCORE_LIBRARY})

add_executable(qmapint qmapint.cpp)
target_link_libraries(qmapint ${QT_QTCORE_LIBRARY})

add_executable(qmapstring qmapstring.cpp)
target_link_libraries(qmapstring ${QT_QTCORE_LIBRARY})

add_executable(qmapstringbool qmapstringbool.cpp)
target_link_libraries(qmapstringbool ${QT_QTCORE_LIBRARY})

add_executable(qdate qdate.cpp)
target_link_libraries(qdate ${QT_QTCORE_LIBRARY})

add_executable(qtime qtime.cpp)
target_link_libraries(qtime ${QT_QTCORE_LIBRARY})

add_executable(qdatetime qdatetime.cpp)
target_link_libraries(qdatetime ${QT_QTCORE_LIBRARY})

add_executable(qurl qurl.cpp)
target_link_libraries(qurl ${QT_QTCORE_LIBRARY})

add_executable(qhashint qhashint.cpp)
target_link_libraries(qhashint ${QT_QTCORE_LIBRARY})

add_executable(qhashstring qhashstring.cpp)
target_link_libraries(qhashstring ${QT_QTCORE_LIBRARY})

add_executable(qsetint qsetint.cpp)
target_link_libraries(qsetint ${QT_QTCORE_LIBRARY})

add_executable(qsetstring qsetstring.cpp)
target_link_libraries(qsetstring ${QT_QTCORE_LIBRARY})

add_executable(qchar qchar.cpp)
target_link_libraries(qchar ${QT_QTCORE_LIBRARY})

add_executable(quuid quuid.cpp)
target_link_libraries(quuid ${QT_QTCORE_LIBRARY})

add_executable(qlistpod qlistpod.cpp)
target_link_libraries(qlistpod ${QT_QTCORE_LIBRARY})

add_executable(ktexteditortypes ktexteditortypes.cpp)
target_link_libraries(ktexteditortypes ${QT_QTCORE_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS})

add_executable(kdeveloptypes kdeveloptypes.cpp)
target_link_libraries(kdeveloptypes ${QT_QTCORE_LIBRARY} ${KDEVPLATFORM_UTIL_LIBRARIES})

kde4_add_unit_test(qtprinters qtprinters.cpp)
target_link_libraries(qtprinters
    ${QT_QTCORE_LIBRARY}
    ${QT_QTTEST_LIBRARY}
)
