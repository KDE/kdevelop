/*****************************************************************************
 *   Copyright 2012 Sven Brauch <svenbrauch@googlemail.com>                  *
 *   Copyright 2014 Denis Steckelmacher <steckdenis@yahoo.fr>                *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>    *
 *****************************************************************************/

#include "qmlcompletiontest.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchain.h>
#include <language/codegen/coderepresentation.h>
#include <language/codecompletion/codecompletiontesthelper.h>
#include <language/codecompletion/codecompletioncontext.h>
#include <language/backgroundparser/backgroundparser.h>

#include <interfaces/ilanguagecontroller.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

#include <QtTest/QTest>

#include "codecompletion/context.h"
#include "codecompletion/model.h"

using namespace KDevelop;
using namespace QmlJS;

QTEST_MAIN(QmlJS::QmlCompletionTest)

using CompletionContextPtr = QSharedPointer<QmlJS::CodeCompletionContext>;

namespace {

struct CompletionParameters
{
    QSharedPointer<TestFile> file;
    DUContextPointer contextAtCursor;
    QString snip;
    QString remaining;
    CursorInRevision cursorAt;

    CompletionContextPtr completionContext;
    QList<CompletionTreeItemPointer> completionItems;
};

void runCompletion(CompletionParameters* parameters)
{
    parameters->completionContext = CompletionContextPtr(new QmlJS::CodeCompletionContext(parameters->contextAtCursor,
                                                               parameters->snip,
                                                               parameters->cursorAt));
    bool abort = false;

    parameters->completionItems = parameters->completionContext->completionItems(abort, true);
}

CompletionParameters prepareCompletion(const QString& initCode, const QString& invokeCode, bool qml)
{
    CompletionParameters completion_data;

    // Simulate that the user has entered invokeCode where %INVOKE is, put
    // the cursor where %CURRSOR is, and then asked for completions
    Q_ASSERT(initCode.indexOf("%INVOKE") != -1);

    // Create a file containing the given code, with "%INVOKE" removed
    completion_data.file = QSharedPointer<TestFile>(new TestFile(QString(initCode).replace("%INVOKE", ""),
                                                    qml ? "qml" : "js"));

    completion_data.file->parse();
    completion_data.file->waitForParsed();
    // wait for this fail and all dependencies, like modules and such
    while (!ICore::self()->languageController()->backgroundParser()->isIdle()) {
        QTest::qWait(100);
    }

    if (!completion_data.file->topContext()) {
      qWarning() << "file contents are: " << completion_data.file->fileContents();
      Q_ASSERT_X(false, Q_FUNC_INFO, "Failed to parse initCode.");
    }

    QString allCode = QString(initCode).replace("%INVOKE", invokeCode);

    QStringList lines = allCode.split('\n');
    completion_data.cursorAt = CursorInRevision::invalid();
    for ( int i = 0; i < lines.length(); i++ ) {
        int j = lines.at(i).indexOf("%CURSOR");
        if ( j != -1 ) {
            completion_data.cursorAt = CursorInRevision(i, j);
            break;
        }
    }
    Q_ASSERT(completion_data.cursorAt.isValid());

    // codeCompletionContext only gets passed the text until the place where completion is invoked
    completion_data.snip = allCode.mid(0, allCode.indexOf("%CURSOR"));
    completion_data.remaining = allCode.mid(allCode.indexOf("%CURSOR") + 7);

    DUChainReadLocker lock;
    completion_data.contextAtCursor = DUContextPointer(completion_data.file->topContext()->findContextAt(completion_data.cursorAt, true));
    Q_ASSERT(completion_data.contextAtCursor);

    runCompletion(&completion_data);

    return completion_data;
}

bool containsItemForDeclarationNamed(const CompletionParameters& params, const QString& itemName)
{
    DUChainReadLocker lock;

    foreach (const CompletionTreeItemPointer& ptr, params.completionItems) {
        if (ptr->declaration()) {
            if (ptr->declaration()->identifier().toString() == itemName) {
                return true;
            }
        }
    }

    qWarning() << "could not find declaration with name" << itemName;
    return false;
}

bool declarationInCompletionList(const QString& initCode, const QString& invokeCode, QString itemName, bool qml)
{
    return containsItemForDeclarationNamed(
        prepareCompletion(initCode, invokeCode, qml),
        itemName
    );
}

}

namespace QmlJS {

void QmlCompletionTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    DUChain::self()->disablePersistentStorage();
    CodeRepresentation::setDiskChangesForbidden(true);
}

void QmlCompletionTest::cleanupTestCase()
{
  TestCore::shutdown();
}

void QmlCompletionTest::testContainsDeclaration()
{
    QFETCH(QString, invokeCode);
    QFETCH(QString, completionCode);
    QFETCH(QString, expectedItem);
    QFETCH(bool, qml);

    QVERIFY(declarationInCompletionList(invokeCode, completionCode, expectedItem, qml));
}

void QmlCompletionTest::testContainsDeclaration_data()
{
    QTest::addColumn<QString>("invokeCode");
    QTest::addColumn<QString>("completionCode");
    QTest::addColumn<QString>("expectedItem");
    QTest::addColumn<bool>("qml");

    // Basic JS tests
    QTest::newRow("js_basic_variable") << "var a;\n %INVOKE" << "%CURSOR" << "a" << false;
    QTest::newRow("js_basic_function") << "function f();\n %INVOKE" << "%CURSOR" << "f" << false;

    // Object members
    QTest::newRow("js_object_members") << "var a = {b: 0};\n %INVOKE" << "a.%CURSOR" << "b" << false;
    QTest::newRow("js_array_subscript") << "var a = {b: 0};\n %INVOKE" << "a[%CURSOR" << "b" << false;
    QTest::newRow("js_skip_separators") << "var a = {b: 0};\n %INVOKE" << "foo(false, a.%CURSOR" << "b" << false;

    // Basic QML tests
    QTest::newRow("qml_basic_property") << "Item { id: foo\n property int prop\n %INVOKE }" << "%CURSOR" << "prop" << true;
    QTest::newRow("qml_basic_instance") << "Item { id: foo\n onTest: %INVOKE }" << "%CURSOR" << "foo" << true;
    QTest::newRow("qml_skip_separators") << "Item { id: foo\n Item { id: bar\n property int prop }\n %INVOKE" << "onTest: bar.%CURSOR" << "prop" << true;

    // QML inheritance
    QTest::newRow("qml_inheritance") <<
        "Module {\n"
        " Component {\n"
        "  name: \"TestComponent\"\n"
        "  Property {\n"
        "   name: \"prop\"\n"
        "   type: \"int\"\n"
        "  }\n"
        " }\n"
        " TestComponent {\n"
        "  id: foo\n"
        "  %INVOKE\n"
        " }\n"
        "}" << "%CURSOR" << "prop" << true;

    // QML parent
    QTest::newRow("qml_parent") <<
        "Item {\n"
        " id: a\n"
        " property var prop\n"
        " Item {\n"
        "  id: b\n"
        "  %INVOKE\n"
        " }\n"
        "}\n" << "onTest: parent.%CURSOR" << "prop" << true;

    // This declaration must be in QtQuick 2.2 but not 2.0 (tested in testDoesNotContainDeclaration)
    QTest::newRow("qml_module_version_2.2") << "import QtQuick 2.2\n Item { id: a\n %INVOKE }" << "%CURSOR" << "OpacityAnimator" << true;
}

void QmlCompletionTest::testDoesNotContainDeclaration()
{
    QFETCH(QString, invokeCode);
    QFETCH(QString, completionCode);
    QFETCH(QString, item);
    QFETCH(bool, qml);

    QVERIFY(!declarationInCompletionList(invokeCode, completionCode, item, qml));
}

void QmlCompletionTest::testDoesNotContainDeclaration_data()
{
    QTest::addColumn<QString>("invokeCode");
    QTest::addColumn<QString>("completionCode");
    QTest::addColumn<QString>("item");
    QTest::addColumn<bool>("qml");

    // Don't show unreachable declarations when providing code-completions for object members
    QTest::newRow("js_object_member_not_surrounding") << "var a; var b = {c: 0};%INVOKE" << "b.%CURSOR" << "a" << false;
    QTest::newRow("js_object_member_local") << "var a = {b: 0};%INVOKE" << "%CURSOR" << "b" << false;

    // When providing completions for script bindings, don't propose script bindings
    // for properties/signals of the surrounding components
    QTest::newRow("qml_script_binding_not_surrounding") << "Item { property int foo; Item { %INVOKE } }" << "%CURSOR" << "foo" << false;

    // Some QML components are only available in specific versions of their module
    QTest::newRow("qml_module_version_2.0") << "import QtQuick 2.0\n Item { id: a\n %INVOKE }" << "%CURSOR" << "OpacityAnimator" << true;
}

}
