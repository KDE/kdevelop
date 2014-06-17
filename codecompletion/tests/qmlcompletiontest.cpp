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

#include <language/backgroundparser/backgroundparser.h>
#include <language/codecompletion/codecompletiontesthelper.h>
#include <language/duchain/declaration.h>
#include <language/codegen/coderepresentation.h>
#include <language/duchain/duchain.h>
#include <interfaces/ilanguagecontroller.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <QtTest/QTest>

#include "codecompletion/context.h"

using namespace KDevelop;

QTEST_MAIN(QmlJS::QmlCompletionTest)

static int testId = 0;
static QString basepath = "/tmp/__kdevqmljscompletiontest.dir/";
static QFSFileEngine fileEngine;

namespace QmlJS {

QString filenameForTestId(const int id, bool qml) {
    return basepath + "test" + QString::number(id) + (qml ? ".qml" : ".js");
}

QString nextFilename(bool qml) {
    testId += 1;
    return filenameForTestId(testId, qml);
}

QmlCompletionTest::QmlCompletionTest(QObject* parent)
: QObject(parent)
{
    initShell();

    QDir::root().mkpath(basepath);
}

void QmlCompletionTest::initShell()
{
    AutoTestShell::init();
    TestCore* core = new TestCore();
    core->initialize(KDevelop::Core::NoUi);
    DUChain::self()->disablePersistentStorage();
    KDevelop::CodeRepresentation::setDiskChangesForbidden(true);
}

const QList<CompletionTreeItem*> QmlCompletionTest::invokeCompletionOn(const QString& initCode, const QString& invokeCode, bool qml)
{
    CompletionParameters data = prepareCompletion(initCode, invokeCode, qml);
    return runCompletion(data);
}

const CompletionParameters QmlCompletionTest::prepareCompletion(const QString& initCode, const QString& invokeCode, bool qml)
{
    CompletionParameters completion_data;

    // Create a file containing the given code, with "%INVOKE" removed
    QString filename = nextFilename(qml);
    QFile fileptr(filename);
    fileptr.open(QIODevice::WriteOnly);
    fileptr.write(initCode.toAscii().replace("%INVOKE", ""));
    fileptr.close();

    DUChain::self()->updateContextForUrl(IndexedString(filename), KDevelop::TopDUContext::ForceUpdate);
    ICore::self()->languageController()->backgroundParser()->parseDocuments();
    ReferencedTopDUContext topContext = DUChain::self()->waitForUpdate(IndexedString(filename),
                                                                       KDevelop::TopDUContext::AllDeclarationsAndContexts);

    while (!ICore::self()->languageController()->backgroundParser()->isIdle()) {
        QTest::qWait(500);
    }

    Q_ASSERT(topContext);

    // Now that it has been parsed, the file can be deleted (this avoids problems
    // with files automatically including each other)
    fileptr.remove();

    // Simulate that the user has entered invokeCode where %INVOKE is, put
    // the cursor where %CURRSOR is, and then asked for completions
    Q_ASSERT(initCode.indexOf("%INVOKE") != -1);
    QString copy = initCode;
    QString allCode = copy.replace("%INVOKE", invokeCode);

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
    completion_data.contextAtCursor = DUContextPointer(topContext->findContextAt(completion_data.cursorAt, true));
    Q_ASSERT(completion_data.contextAtCursor);

    return completion_data;
}

const QList<CompletionTreeItem*> QmlCompletionTest::runCompletion(const CompletionParameters parameters)
{
    CodeCompletionContext* context = new CodeCompletionContext(parameters.contextAtCursor, parameters.snip, parameters.cursorAt);
    QList<CompletionTreeItem*> items;
    bool abort = false;

    foreach (CompletionTreeItemPointer ptr, context->completionItems(abort, true)) {
        items << ptr.data();
        // those are leaked, but it's only a few kb while the tests are running. who cares.
        m_ptrs << ptr;
    }

    return items;
}

bool QmlCompletionTest::containsItemForDeclarationNamed(const QList<CompletionTreeItem*> items, QString itemName)
{
    DUChainReadLocker lock;

    foreach (const CompletionTreeItem* ptr, items) {
        if (ptr->declaration()) {
            if (ptr->declaration()->identifier().toString() == itemName) {
                return true;
            }
        }
    }

    return false;
}

bool QmlCompletionTest::declarationInCompletionList(const QString& initCode, const QString& invokeCode, QString itemName, bool qml)
{
    return containsItemForDeclarationNamed(
        invokeCompletionOn(initCode, invokeCode, qml),
        itemName
    );
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
