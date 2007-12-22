
set(parser_STAT_SRCS
    qmakeast.cpp
    qmakeastassignment.cpp
    qmakeastbody.cpp
    qmakeastor.cpp
    qmakeastproject.cpp
    qmakeastfunc.cpp
    qmakeastscope.cpp
    qmakeastsimple.cpp
    qmakedriver.cpp
    qmakedebugvisitor.cpp
    buildastvisitor.cpp
    qmakelexer.cpp
    qmakeastvisitor.cpp
    qmakeastdefaultvisitor.cpp
    )

if(KDEVPGQT_FOUND)
    kdevpgqt_generate(parser_GEN_SRCS qmake NAMESPACE QMake DUMP_INFO
        "${QMakeManager_SOURCE_DIR}/parser/qmake.g"
        "${QMakeManager_SOURCE_DIR}/parser/qmakelexer.h")

    include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KDEVPGQT_INCLUDE_DIR} )

else(KDEVPGQT_FOUND)
    message(STATUS "Assuming existence of the generated parser files for the parser")
    set(parser_GEN_SRCS
        generated/qmakeparser.cpp
        generated/qmakedefaultvisitor.cpp
        generated/qmakevisitor.cpp)
    include_directories( generated
        generated/kdevelop-pg-qt )
endif(KDEVPGQT_FOUND)

add_subdirectory(tests)

add_executable(qmake-parser main.cpp)
target_link_libraries(qmake-parser kdev4qmakeparser )

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
kde4_add_library( kdev4qmakeparser SHARED ${parser_STAT_SRCS} ${parser_GEN_SRCS})
target_link_libraries(kdev4qmakeparser ${KDE4_KDECORE_LIBS})

install(TARGETS qmake-parser DESTINATION ${BIN_INSTALL_DIR})
install(TARGETS kdev4qmakeparser DESTINATION ${LIB_INSTALL_DIR})

add_custom_target( copy-generated
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakeast.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakeast.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakeparser.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakeparser.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakevisitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakevisitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakevisitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakevisitor.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakedefaultvisitor.h" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakedefaultvisitor.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/qmakedefaultvisitor.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/generated/qmakedefaultvisitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakeast.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakeparser.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakedefaultvisitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakedefaultvisitor.cpp"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakevisitor.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmakevisitor.h"
    )

