add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

set(KDevPlatformSerialization_LIB_SRCS
    abstractitemrepository.cpp
    indexedstring.cpp
    itemrepositoryregistry.cpp
    referencecounting.cpp
)

declare_qt_logging_category(KDevPlatformSerialization_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "serialization"
)

kdevplatform_add_library(KDevPlatformSerialization SOURCES ${KDevPlatformSerialization_LIB_SRCS})
target_link_libraries(KDevPlatformSerialization
PUBLIC
    KF6::WidgetsAddons
    KF6::I18n
PRIVATE
    KDev::Util
)

install(FILES
    abstractitemrepository.h
    referencecounting.h
    indexedstring.h
    indexedstringview.h
    itemrepositoryexampleitem.h
    itemrepository.h
    itemrepositoryregistry.h
    itemrepositoryreferencecounting.h
    repositorymanager.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/serialization COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
