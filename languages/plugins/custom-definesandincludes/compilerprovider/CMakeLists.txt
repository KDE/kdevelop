set( compilerprovider_SRCS
        compilerprovider.cpp
        icompiler.cpp
        gcclikecompiler.cpp
        msvccompiler.cpp
        compilerfactories.cpp
        settingsmanager.cpp
        ../debugarea.cpp
    )

add_library(kdevcompilerprovider STATIC
        ${compilerprovider_SRCS})

target_link_libraries( kdevcompilerprovider LINK_PRIVATE
        KDev::Project
        KDev::Util
        KDev::Language )

option(BUILD_kdev_msvcdefinehelper "Build the msvcdefinehelper tool for retrieving msvc standard macro definitions" OFF)

if(BUILD_kdev_msvcdefinehelper)
    set(kdevmsvcdefinehelper_SRCS msvcdefinehelper.cpp)
    add_executable(kdevmsvcdefinehelper ${kdevmsvcdefinehelper_SRCS})
    install(TARGETS kdevmsvcdefinehelper RUNTIME DESTINATION bin
                                         LIBRARY DESTINATION lib)
endif(BUILD_kdev_msvcdefinehelper)

add_subdirectory(tests)
