remove_definitions( -DKDE_DEFAULT_DEBUG_AREA=9007 ) 
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

include_directories(
    ${cpp_SOURCE_DIR}
    ${cpp_BINARY_DIR})

add_subdirectory(tests)

########### next target ###############

set(veritascppsupport_LIB_SRCS
    testswitch.cpp
    classskeleton.cpp
    constructorskeleton.cpp
    methodskeleton.cpp
    stubconstructor.cpp
    classwriter.cpp
    stubcontextaction.cpp
    uutconstructor.cpp
    documentaccess.cpp
    uutcontextaction.cpp
    tests/classdeclarationfactory.cpp
)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target

kde4_add_library(veritascpp SHARED ${veritascppsupport_LIB_SRCS})
target_link_libraries(veritascpp
    kdev4cppduchain
    kdev4cppparser
    kdev4cpprpp
    ${KDEVPLATFORM_LANGUAGE_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS})
install(TARGETS veritascpp ${INSTALL_TARGETS_DEFAULT_ARGS})

