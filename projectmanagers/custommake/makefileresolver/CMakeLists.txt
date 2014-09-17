set(makefileresolver_SRCS
    makefileresolver.cpp
)

add_library(kdev4makefileresolver STATIC
    ${makefileresolver_SRCS}
)
target_link_libraries(kdev4makefileresolver
  KDev::Interfaces KDev::Project
  Qt5::Core Qt5::Gui KF5::KDELibs4Support KDev::Language
  KDev::Tests
)
set_target_properties(kdev4makefileresolver PROPERTIES POSITION_INDEPENDENT_CODE ON)

option(BUILD_kdev_makefileresolver "Build the makefile resolver debugging tool" OFF)
if(BUILD_kdev_makefileresolver)
    add_executable(kdev_makefileresolver makefileresolver.cpp)
    set_target_properties( kdev_makefileresolver PROPERTIES COMPILE_FLAGS -DTEST )
    target_link_libraries(kdev_makefileresolver kdev4makefileresolver)

    install(TARGETS kdev_makefileresolver ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif()

add_subdirectory(tests)
