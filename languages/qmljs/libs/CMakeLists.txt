qt5_wrap_ui(uifiles_SRCS
	utils/projectintropage.ui
	utils/filewizardpage.ui
	utils/submiteditorwidget.ui
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

if (APPLE) 
    target_sources(kdevqtc-qmlsupport PRIVATE utils/fileutils_mac.mm)
    target_link_libraries(kdevqtc-qmlsupport LINK_PRIVATE "-framework Foundation" )
endif()

set_target_properties(kdevqtc-qmlsupport PROPERTIES COMPILE_FLAGS "-DLANGUAGEUTILS_BUILD_DIR\ -DQTCREATOR_UTILS_LIB")
target_link_libraries(kdevqtc-qmlsupport LINK_PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Xml
)
