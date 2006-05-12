


add_definitions(-fPIC -DPP_WITH_MACRO_POSITION)

########### next target ###############

set(kdevcpprpp_STAT_SRCS
    pp-stream.cpp
    pp-macro-expander.cpp
    pp-scanner.cpp
    pp-macro.cpp
    pp-engine.cpp
    pp-internal.cpp
    preprocessor.cpp
)

kde4_automoc(${kdevcpprpp_STAT_SRCS})
kde4_add_library(kdevcpprpp STATIC ${kdevcpprpp_STAT_SRCS})

########### next target ###############

if(KDE4_BUILD_TESTS)

set(pp_SRCS
    main.cpp
)

kde4_add_executable(pp ${pp_SRCS})
target_link_libraries(pp ${KDE4_KDECORE_LIBS} kdevcpprpp)
add_test(pp ${EXECUTABLE_OUTPUT_PATH}/pp)

endif(KDE4_BUILD_TESTS)


########### install files ###############


