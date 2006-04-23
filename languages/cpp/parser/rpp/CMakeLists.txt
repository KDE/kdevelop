kde4_header()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-fPIC -DPP_WITH_MACRO_POSITION)

########### next target ###############

set(kdevcpprpp_STAT_SRCS
    preprocessor.cpp
)

kde4_automoc(${kdevcpprpp_STAT_SRCS})
kde4_add_library(kdevcpprpp STATIC ${kdevcpprpp_STAT_SRCS})

########### next target ###############

if(KDE4_BUILD_TESTS)

set(pp_SRCS
    preprocessor.cpp
    main.cpp
)

kde4_add_executable(pp ${pp_SRCS})
target_link_libraries(pp ${KDE4_KDECORE_LIBS})
add_test(pp ${EXECUTABLE_OUTPUT_PATH}/pp)

endif(KDE4_BUILD_TESTS)


########### install files ###############


kde4_footer()
