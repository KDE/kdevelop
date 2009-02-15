
add_subdirectory( tests )

########### next target ###############

set(kdevcpprpp_LIB_SRCS
    pp-stream.cpp
    pp-macro-expander.cpp
    pp-scanner.cpp
    pp-macro.cpp
    pp-engine.cpp
    pp-internal.cpp
    pp-environment.cpp
    pp-location.cpp
    preprocessor.cpp
    chartools.cpp
    macrorepository.cpp
)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
kde4_add_library(kdev4cpprpp SHARED ${kdevcpprpp_LIB_SRCS})
target_link_libraries(kdev4cpprpp ${KDE4_KDECORE_LIBS} ${KDEVPLATFORM_LANGUAGE_LIBRARY} ${KDE4_KTEXTEDITOR_LIBS} )
install(TARGETS kdev4cpprpp ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

# install(FILES
#     pp-engine.h
#     pp-internal.h
#     pp-macro-expander.h
#     pp-macro.h
#     pp-scanner.h
#     pp-stream.h
#     pp-environment.h
#     preprocessor.h
#     DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/rpp
# )
