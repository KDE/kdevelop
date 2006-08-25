
if(KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/languages/cpp/parser ${CMAKE_SOURCE_DIR}/languages/cpp/duchain ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/editor)


########### next target ###############

set(kdevplatform_editor_LIB_SRCS
    ../../../../lib/editor/kdeveditorintegrator.cpp
    ../../../../lib/editor/kdevdocumentrangeobject.cpp
    ../../../../lib/editor/kdevdocumentcursorobject.cpp
    ../../../../lib/editor/kdevdocumentcursor.cpp
    ../../../../lib/editor/kdevdocumentrange.cpp
)

set(kdevcppduchain_SRCS
    ../../duchain/cppeditorintegrator.cpp
    ../../duchain/duchain.cpp
    ../../duchain/definitionbuilder.cpp
    ../../duchain/usebuilder.cpp
    ../../duchain/ducontext.cpp
    ../../duchain/topducontext.cpp
    ../../duchain/definition.cpp
    ../../duchain/definitionuse.cpp
    ../../duchain/dumpchain.cpp
)

set(duchaintest_SRCS ${kdevplatform_editor_LIB_SRCS} ${kdevcppduchain_SRCS} test_duchain.cpp)
kde4_automoc(${kdevplatform_editor_LIB_SRCS} ${kdevcppduchain_SRCS})

kdevelop_add_test(${duchaintest_SRCS})
kde4_add_executable(duchaintest ${duchaintest_SRCS})
target_link_libraries(duchaintest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdevcppparser kdevcpprpp ktexteditor)
add_test(kdevelop-duchaintest ${EXECUTABLE_OUTPUT_PATH}/duchaintest)

endif(KDE4_BUILD_TESTS)

