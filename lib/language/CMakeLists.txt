include_directories(
    ${CMAKE_SOURCE_DIR}/lib/
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language/interfaces
    ${CMAKE_SOURCE_DIR}/lib/editor
)

set(kdevplatformlanguage_LIB_SRCS
    backgroundparser/backgroundparser.cpp
    backgroundparser/parsejob.cpp
    backgroundparser/parserdependencypolicy.cpp
)

kde4_automoc(${kdevplatformlanguage_LIB_SRCS})

kde4_add_library(kdevplatformlanguage SHARED ${kdevplatformlanguage_LIB_SRCS})
target_link_libraries(kdevplatformlanguage ${KDE4_KPARTS_LIBS} ktexteditor ${KDE4_THREADWEAVER_LIBRARIES}
    kdevplatforminterfaces kdevplatformeditor)
set_target_properties(kdevplatformlanguage PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformlanguage DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    interfaces/ilanguagesupport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/language/interfaces
)

install(FILES
    backgroundparser/backgroundparser.h
    backgroundparser/parsejob.h
    backgroundparser/parserdependencypolicy.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/language/backgroundparser
)
