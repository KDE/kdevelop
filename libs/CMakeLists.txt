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
# 	utils/annotateditemdelegate.cpp
# 	utils/appmainwindow.cpp
# 	utils/basetreeview.cpp
# 	utils/basevalidatinglineedit.cpp
# 	utils/bracematcher.cpp
# 	utils/buildablehelperlibrary.cpp
	utils/changeset.cpp
# 	utils/checkablemessagebox.cpp
# 	utils/classnamevalidatinglineedit.cpp
# 	utils/codegeneration.cpp
# 	utils/completingtextedit.cpp
# 	utils/consoleprocess.cpp
# 	utils/consoleprocess_unix.cpp
# 	utils/crumblepath.cpp
# 	utils/detailsbutton.cpp
# 	utils/detailswidget.cpp
# 	utils/elfreader.cpp
# 	utils/environment.cpp
# 	utils/environmentmodel.cpp
# 	utils/faketooltip.cpp
# 	utils/fancylineedit.cpp
# 	utils/fancymainwindow.cpp
# 	utils/fileinprojectfinder.cpp
# 	utils/filenamevalidatinglineedit.cpp
# 	utils/filesearch.cpp
# 	utils/filesystemwatcher.cpp
	utils/fileutils.cpp
# 	utils/filewizarddialog.cpp
# 	utils/filewizardpage.cpp
# 	utils/filterlineedit.cpp
# 	utils/flowlayout.cpp
# 	utils/historycompleter.cpp
# 	utils/htmldocextractor.cpp
# 	utils/ipaddresslineedit.cpp
# 	utils/iwelcomepage.cpp
	utils/json.cpp
# 	utils/linecolumnlabel.cpp
# 	utils/navigationtreeview.cpp
# 	utils/networkaccessmanager.cpp
# 	utils/newclasswidget.cpp
# 	utils/outputformatter.cpp
# 	utils/parameteraction.cpp
# 	utils/pathchooser.cpp
# 	utils/pathlisteditor.cpp
# 	utils/persistentsettings.cpp
# 	utils/projectintropage.cpp
# 	utils/projectnamevalidatinglineedit.cpp
# 	utils/proxyaction.cpp
	utils/qtcassert.cpp
# 	utils/qtcolorbutton.cpp
# 	utils/qtcprocess.cpp
# 	utils/reloadpromptutils.cpp
# 	utils/savedaction.cpp
	utils/savefile.cpp
# 	utils/settingsselector.cpp
# 	utils/statuslabel.cpp
# 	utils/stringutils.cpp
# 	utils/styledbar.cpp
# 	utils/stylehelper.cpp
# 	utils/submiteditorwidget.cpp
# 	utils/submitfieldwidget.cpp
# 	utils/synchronousprocess.cpp
# 	utils/tcpportsgatherer.cpp
# 	utils/treewidgetcolumnstretcher.cpp
# 	utils/uncommentselection.cpp
# 	utils/unixutils.cpp
# 	utils/wizard.cpp
)
target_link_libraries(kdevqtc-qmlsupport LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)

install(TARGETS kdevqtc-qmlsupport ${INSTALL_TARGETS_DEFAULT_ARGS} )
