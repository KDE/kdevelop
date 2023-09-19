remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_FOREACH
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
)

qt_wrap_ui(uifiles_SRCS
	utils/projectintropage.ui
	utils/filewizardpage.ui
	utils/newclasswidget.ui
)

add_library(kdevqtc-qmlsupport STATIC
	${uifiles_SRCS}
	languageutils/componentversion.cpp
	languageutils/fakemetaobject.cpp
	qmljs/parser/qmldirparser.cpp
	qmljs/parser/qmlerror.cpp
	qmljs/parser/qmljsast.cpp
	qmljs/parser/qmljsastvisitor.cpp
	qmljs/parser/qmljsengine_p.cpp
	qmljs/parser/qmljsgrammar.cpp
	qmljs/parser/qmljslexer.cpp
	qmljs/parser/qmljsparser.cpp
	qmljs/qmljsdocument.cpp
	qmljs/qmljsutils.cpp
	utils/changeset.cpp
	utils/fileutils.cpp
	utils/qtcassert.cpp
	utils/savefile.cpp
	utils/json.cpp
	utils/filesystemwatcher.cpp
	utils/environment.cpp
	utils/hostosinfo.cpp
	utils/runextensions.cpp

    qmljs/persistenttrie.cpp
    qmljs/qmljsbind.cpp
    qmljs/qmljsbundle.cpp
    qmljs/qmljscontext.cpp
    qmljs/qmljsdialect.cpp
    qmljs/qmljsevaluate.cpp
    qmljs/qmljsimportdependencies.cpp
    qmljs/qmljsinterpreter.cpp
    qmljs/qmljsmodelmanagerinterface.cpp
    qmljs/qmljsplugindumper.cpp
    qmljs/qmljsqrcparser.cpp
    qmljs/qmljsscopeastpath.cpp
    qmljs/qmljsscopebuilder.cpp
    qmljs/qmljsscopechain.cpp
    qmljs/qmljstypedescriptionreader.cpp
    qmljs/qmljsvalueowner.cpp
    qmljs/qmljsviewercontext.cpp
)

target_compile_options(kdevqtc-qmlsupport PRIVATE -UQT_DISABLE_DEPRECATED_BEFORE)

if (APPLE) 
    target_sources(kdevqtc-qmlsupport PRIVATE utils/fileutils_mac.mm)
    target_link_libraries(kdevqtc-qmlsupport PRIVATE "-framework Foundation" )
endif()

# silence warnings which won't be fixed in this 3rd-party code copy, to keep diff small
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-documentation")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-pedantic")
    add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-implicit-fallthrough")
    add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-overloaded-virtual")
    add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-zero-as-null-pointer-constant")
    add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-deprecated-declarations")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-suggest-override")
   add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-class-memaccess")
   add_target_compile_flag_if_supported(kdevqtc-qmlsupport PRIVATE "-Wno-deprecated-copy")
endif()
if (MSVC)
    target_compile_options(kdevqtc-qmlsupport PRIVATE "/wd4996") # C4996: warns about deprecated declarations
endif()

target_compile_definitions(kdevqtc-qmlsupport PUBLIC -DLANGUAGEUTILS_LIBRARY -DUTILS_LIBRARY -DQT_CREATOR -DQML_BUILD_STATIC_LIB)
# add as SYSTEM include dir so compiler does not emit warnings for the lib headers
target_include_directories(kdevqtc-qmlsupport SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# optional.hpp needs -fexceptions
# (otherwise: plugins/qmljs/3rdparty/qtcreator-libs/3rdparty/optional/optional.hpp:561:94: error: exception handling disabled, use -fexceptions to enable)
kde_target_enable_exceptions(kdevqtc-qmlsupport PRIVATE)
target_link_libraries(kdevqtc-qmlsupport
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Widgets
        Qt${QT_MAJOR_VERSION}::Network
        Qt${QT_MAJOR_VERSION}::Xml
)
