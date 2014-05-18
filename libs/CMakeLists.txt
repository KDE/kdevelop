include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

kde4_add_ui_files(uifiles_SRCS
	utils/projectintropage.ui
	utils/filewizardpage.ui
	utils/submiteditorwidget.ui
	utils/newclasswidget.ui
)

kde4_add_library(kdevqtc-qmlsupport SHARED
	${uifiles_SRCS}
	languageutils/componentversion.cpp
	languageutils/fakemetaobject.cpp
	qmljs/consoleitem.cpp
	qmljs/consolemanagerinterface.cpp
	qmljs/jsoncheck.cpp
	qmljs/parser/qmldirparser.cpp
	qmljs/parser/qmlerror.cpp
	qmljs/parser/qmljsast.cpp
	qmljs/parser/qmljsastvisitor.cpp
	qmljs/parser/qmljsengine_p.cpp
	qmljs/parser/qmljsgrammar.cpp
	qmljs/parser/qmljslexer.cpp
	qmljs/parser/qmljsparser.cpp
	qmljs/qmljsbind.cpp
	qmljs/qmljscheck.cpp
	qmljs/qmljscodeformatter.cpp
	qmljs/qmljscompletioncontextfinder.cpp
	qmljs/qmljscontext.cpp
	qmljs/qmljsdelta.cpp
	qmljs/qmljsdocument.cpp
	qmljs/qmljsevaluate.cpp
	qmljs/qmljsicons.cpp
	qmljs/qmljsindenter.cpp
	qmljs/qmljsinterpreter.cpp
	qmljs/qmljslineinfo.cpp
	qmljs/qmljslink.cpp
	qmljs/qmljsmodelmanagerinterface.cpp
	qmljs/qmljspropertyreader.cpp
	qmljs/qmljsreformatter.cpp
	qmljs/qmljsrewriter.cpp
	qmljs/qmljsscanner.cpp
	qmljs/qmljsscopeastpath.cpp
	qmljs/qmljsscopebuilder.cpp
	qmljs/qmljsscopechain.cpp
	qmljs/qmljsstaticanalysismessage.cpp
	qmljs/qmljstypedescriptionreader.cpp
	qmljs/qmljsutils.cpp
	qmljs/qmljsvalueowner.cpp
	utils/changeset.cpp
	utils/fileutils.cpp
	utils/json.cpp
	utils/qtcassert.cpp
	utils/savefile.cpp
)
target_link_libraries(kdevqtc-qmlsupport LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)

install(TARGETS kdevqtc-qmlsupport ${INSTALL_TARGETS_DEFAULT_ARGS} )
