project(weaver)

find_package(KDE4 REQUIRED)

include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DUSE_CMAKE)

########### next target ###############

set(ThreadWeaver_LIB_SRCS
    WeaverInterface.cpp
    ThreadWeaver.cpp
    WeaverImpl.cpp
    DebuggingAids.cpp
    Thread.cpp
    Job.cpp
    State.cpp
    StateImplementation.cpp
    InConstructionState.cpp
    WorkingHardState.cpp
    SuspendingState.cpp
    SuspendedState.cpp
    ShuttingDownState.cpp
    DestructedState.cpp
    WeaverObserver.cpp
    JobCollection.cpp
    JobSequence.cpp
    DependencyPolicy.cpp
    ResourceRestrictionPolicy.cpp
)

qt4_generate_moc(${CMAKE_CURRENT_SOURCE_DIR}/Job.cpp ${CMAKE_CURRENT_BINARY_DIR}/Job_moc.cpp)
qt4_generate_moc(${CMAKE_CURRENT_SOURCE_DIR}/Thread.cpp ${CMAKE_CURRENT_BINARY_DIR}/Thread_moc.cpp)
qt4_generate_moc(${CMAKE_CURRENT_SOURCE_DIR}/JobCollection.cpp ${CMAKE_CURRENT_BINARY_DIR}/JobCollection_moc.cpp)

QT4_AUTOMOC(${ThreadWeaver_LIB_SRCS})

kde4_add_library(ThreadWeaver SHARED ${ThreadWeaver_LIB_SRCS})
target_link_libraries(ThreadWeaver ${QT_QTCORE_LIBRARY})
set_target_properties(ThreadWeaver PROPERTIES VERSION 4.2.0 SOVERSION 4)
install(TARGETS  ThreadWeaver DESTINATION ${LIB_INSTALL_DIR})

#macro_additional_clean_files(${CMAKE_CURRENT_BINARY_DIR}/Job_moc.cpp ${CMAKE_CURRENT_BINARY_DIR}/Thread_moc.cpp)
