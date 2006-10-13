


add_definitions(-fPIC -DPP_WITH_MACRO_POSITION)

########### next target ###############

set(kdevcpprpp_LIB_SRCS
    pp-stream.cpp
    pp-macro-expander.cpp
    pp-scanner.cpp
    pp-macro.cpp
    pp-engine.cpp
    pp-internal.cpp
    preprocessor.cpp
)

kde4_automoc(kdevcpprpp ${kdevcpprpp_LIB_SRCS})
kde4_add_library(kdevcpprpp SHARED ${kdevcpprpp_LIB_SRCS})
target_link_libraries(kdevcpprpp kdevplatform )
set_target_properties(kdevcpprpp PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevcpprpp DESTINATION ${LIB_INSTALL_DIR} )

########### next target ###############

if(KDE4_BUILD_TESTS)

set(pp_SRCS
    main.cpp
)

kde4_add_executable(pp NOGUI RUN_UNINSTALLED ${pp_SRCS})
target_link_libraries(pp ${KDE4_KDECORE_LIBS} kdevcpprpp)
add_test(pp ${EXECUTABLE_OUTPUT_PATH}/pp)

endif(KDE4_BUILD_TESTS)


########### install files ###############

install(FILES
    pp-engine.h
    pp-internal.h
    pp-macro-expander.h
    pp-macro.h
    pp-scanner.h
    pp-stream.h
    preprocessor.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/rpp
)
