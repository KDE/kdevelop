set(kdevplatformserialization_LIB_SRCS
    abstractitemrepository.cpp
    indexedstring.cpp
    itemrepositoryregistry.cpp
    referencecounting.cpp
)

add_library(kdevplatformserialization SHARED ${kdevplatformserialization_LIB_SRCS})
generate_export_header(kdevplatformserialization EXPORT_FILE_NAME serializationexport.h)

target_link_libraries(kdevplatformserialization
LINK_PUBLIC
    kdevplatforminterfaces
LINK_PRIVATE
    KF5::KDELibs4Support
    kdevplatformutil
)

set_target_properties(kdevplatformserialization PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformserialization EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/serializationexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/serialization COMPONENT Devel
)

install(FILES
    abstractitemrepository.h
    referencecounting.h
    indexedstring.h
    itemrepositoryexampleitem.h
    itemrepository.h
    itemrepositoryregistry.h
    repositorymanager.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/serialization COMPONENT Devel
)

add_subdirectory(tests)
