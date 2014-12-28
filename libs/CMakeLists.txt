include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

qt5_wrap_ui(uifiles_SRCS
	utils/projectintropage.ui
	utils/filewizardpage.ui
	utils/submiteditorwidget.ui
	utils/newclasswidget.ui
)

add_library(kdevqtc-qmlsupport SHARED
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
)

set_target_properties(kdevqtc-qmlsupport PROPERTIES COMPILE_FLAGS "-DLANGUAGEUTILS_BUILD_DIR\ -DQMLJS_BUILD_DIR\ -DQTCREATOR_UTILS_LIB")
target_link_libraries(kdevqtc-qmlsupport LINK_PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${QT_QTSCRIPT_LIBRARY}
    Qt5::Network
)

install(TARGETS kdevqtc-qmlsupport ${INSTALL_TARGETS_DEFAULT_ARGS})
