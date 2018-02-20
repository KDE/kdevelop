add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

set(KDevPlatformSerialization_LIB_SRCS
    abstractitemrepository.cpp
    indexedstring.cpp
    itemrepositoryregistry.cpp
    referencecounting.cpp
)

ecm_qt_declare_logging_category(KDevPlatformSerialization_LIB_SRCS
    HEADER debug.h
    IDENTIFIER SERIALIZATION
    CATEGORY_NAME "kdevplatform.serialization"
)

kdevplatform_add_library(KDevPlatformSerialization SOURCES ${KDevPlatformSerialization_LIB_SRCS})
target_link_libraries(KDevPlatformSerialization
PUBLIC
    KF5::WidgetsAddons
    KF5::I18n
PRIVATE
    KDev::Util
)

install(FILES
    abstractitemrepository.h
    referencecounting.h
    indexedstring.h
    itemrepositoryexampleitem.h
    itemrepository.h
    itemrepositoryregistry.h
    repositorymanager.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/serialization COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
