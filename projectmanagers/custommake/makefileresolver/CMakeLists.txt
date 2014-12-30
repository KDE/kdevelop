set(makefileresolver_SRCS
    makefileresolver.cpp
    helper.cpp
)

add_library(kdevmakefileresolver STATIC
    ${makefileresolver_SRCS}
)
target_link_libraries(kdevmakefileresolver
  KDev::Interfaces KDev::Project
  Qt5::Core Qt5::Gui KDev::Language
  KDev::Tests
)
set_target_properties(kdevmakefileresolver PROPERTIES POSITION_INDEPENDENT_CODE ON)

option(BUILD_kdev_makefileresolver "Build the makefile resolver debugging tool" OFF)
if(BUILD_kdev_makefileresolver)
    add_executable(kdev_makefileresolver makefileresolver.cpp)
    set_target_properties( kdev_makefileresolver PROPERTIES COMPILE_FLAGS -DTEST )
    target_link_libraries(kdev_makefileresolver kdevmakefileresolver)

    install(TARGETS kdev_makefileresolver ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif()

add_subdirectory(tests)
