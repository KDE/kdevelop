set(KDevPlatformSerialization_LIB_SRCS
    abstractitemrepository.cpp
    indexedstring.cpp
    itemrepositoryregistry.cpp
    referencecounting.cpp
)

add_library(KDevPlatformSerialization SHARED ${KDevPlatformSerialization_LIB_SRCS})
add_library(KDev::Serialization ALIAS KDevPlatformSerialization)

generate_export_header(KDevPlatformSerialization EXPORT_FILE_NAME serializationexport.h)

target_link_libraries(KDevPlatformSerialization
LINK_PUBLIC
    KDev::Interfaces
    KF5::KDELibs4Support
LINK_PRIVATE
    KDev::Util
)

set_target_properties(KDevPlatformSerialization PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS KDevPlatformSerialization EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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
