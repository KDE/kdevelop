add_definitions(-DTRANSLATION_DOMAIN=\"kdevbazaar\")

set(kdevbazaar_SRCS
    diffjob.cpp
    bzrannotatejob.cpp
    bazaarplugin.cpp
    bazaarpluginmetadata.cpp
    bazaarutils.cpp
    copyjob.cpp
)

kdevplatform_add_plugin(kdevbazaar JSON kdevbazaar.json SOURCES ${kdevbazaar_SRCS})

target_link_libraries(kdevbazaar
    KDev::Vcs
)

add_subdirectory(icons)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
