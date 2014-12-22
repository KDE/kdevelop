add_definitions(-DTRANSLATION_DOMAIN=\"kdevgenericprojectmanager\")

add_subdirectory(test)

########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

kdevplatform_add_plugin(kdevgenericmanager JSON kdevgenericmanager.json SOURCES ${kdevgenericmanager_PART_SRCS})

target_link_libraries(kdevgenericmanager
    KF5::KIOWidgets
    KDev::Project
)
