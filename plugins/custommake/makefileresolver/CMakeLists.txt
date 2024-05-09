set(makefileresolver_SRCS
    makefileresolver.cpp
    helper.cpp
)

add_library(kdevmakefileresolver STATIC
    ${makefileresolver_SRCS}
)
target_link_libraries(kdevmakefileresolver
  KDev::Interfaces KDev::Project
  Qt::Core Qt::Gui KDev::Language
)
target_include_directories(kdevmakefileresolver INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>"
)

option(BUILD_kdev_makefileresolver "Build the makefile resolver debugging tool" OFF)
if(BUILD_kdev_makefileresolver)
    add_executable(kdev_makefileresolver makefileresolver.cpp)
    set_target_properties( kdev_makefileresolver PROPERTIES COMPILE_FLAGS -DTEST )
    target_link_libraries(kdev_makefileresolver kdevmakefileresolver)

    install(TARGETS kdev_makefileresolver ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
