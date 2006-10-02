
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/languages/cpp/parser )


if(KDE4_BUILD_TESTS)

set(headergen_SRCS
    main.cpp
)

kde4_add_executable(headergen ${headergen_SRCS})
target_link_libraries(headergen ${KDE4_KDECORE_LIBS} kdevcpprpp kdevcppparser)
add_test(headergen ${EXECUTABLE_OUTPUT_PATH}/headergen)

endif(KDE4_BUILD_TESTS)
