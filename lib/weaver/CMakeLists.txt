
project(weaver)

include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/lib )

add_definitions(-DUSE_CMAKE)

########### next target ###############

set(ThreadWeaver_LIB_SRCS
    DestructedState.cpp
    WeaverImpl.cpp
    InConstructionState.cpp
    State.cpp
    ShuttingDownState.cpp
    WeaverInterface.cpp
    WeaverObserver.cpp
    SuspendedState.cpp
    WorkingHardState.cpp
    DebuggingAids.cpp
    ThreadWeaver.cpp
    SuspendingState.cpp
    IncludeMocs.cpp
    JobCollection.cpp
    JobSequence.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Job_moc.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Thread_moc.cpp
)

kde4_automoc(Thread.cpp Job.cpp ${ThreadWeaver_LIB_SRCS})


QT4_GET_MOC_INC_DIRS(_moc_INCS)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Job_moc.cpp
    COMMAND ${QT_MOC_EXECUTABLE}
    ARGS ${_moc_INCS} -f ${CMAKE_CURRENT_SOURCE_DIR}/Job.cpp -o ${CMAKE_CURRENT_BINARY_DIR}/Job_moc.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Job.cpp
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Thread_moc.cpp
    COMMAND ${QT_MOC_EXECUTABLE}
    ARGS ${_moc_INCS} -f ${CMAKE_CURRENT_SOURCE_DIR}/Thread.cpp -o ${CMAKE_CURRENT_BINARY_DIR}/Thread_moc.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Thread.cpp
)

kde4_add_library(ThreadWeaver SHARED ${ThreadWeaver_LIB_SRCS})
target_link_libraries(ThreadWeaver ${QT_QTCORE_LIBRARY})
set_target_properties(ThreadWeaver PROPERTIES VERSION 4.2.0 SOVERSION 4)
install(TARGETS ThreadWeaver DESTINATION ${LIB_INSTALL_DIR} )

macro_additional_clean_files(${CMAKE_CURRENT_BINARY_DIR}/Job_moc.cpp ${CMAKE_CURRENT_BINARY_DIR}/Thread_moc.cpp)

