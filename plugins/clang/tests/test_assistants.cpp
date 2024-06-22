/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2014 David Stevens <dgedstevens@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "test_assistants.h"

#include "codegen/clangrefactoring.h"
#include "codegen/adaptsignatureassistant.h"

#include <QTest>
#include <QLoggingCategory>
#include <QTemporaryDir>

#include <KLocalizedString>

#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testfile.h>

#include <util/foregroundlock.h>

#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/isourceformattercontroller.h>

#include <language/assistant/staticassistant.h>
#include <language/assistant/staticassistantsmanager.h>
#include <language/assistant/renameaction.h>
#include <language/assistant/renameassistant.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/codegen/coderepresentation.h>

#include <shell/documentcontroller.h>

#include <clang-c/Index.h>

#include "sanitizer_test_init.h"

using namespace KDevelop;
using namespace KTextEditor;

// TODO: QTEST_MAIN_WRAPPER not part of public documented API
QTEST_MAIN_WRAPPER(TestAssistants, KDevelop::sanitizerTestInit(argv); QApplication app(argc, argv););

ForegroundLock *globalTestLock = nullptr;
StaticAssistantsManager *staticAssistantsManager() { return Core::self()->languageController()->staticAssistantsManager(); }

void TestAssistants::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral(
        "*.debug=false\n"
        "default.debug=true\n"
        "kdevelop.plugins.clang.debug=true\n"
    ));
    QVERIFY(qputenv("KDEV_CLANG_DISPLAY_DIAGS", "1"));
    AutoTestShell::init({QStringLiteral("kdevclangsupport"), QStringLiteral("kdevproblemreporter")});
    TestCore::initialize();
    DUChain::self()->disablePersistentStorage();
    Core::self()->languageController()->backgroundParser()->setDelay(0);
    Core::self()->sourceFormatterController()->disableSourceFormatting();
    CodeRepresentation::setDiskChangesForbidden(true);

    globalTestLock = new ForegroundLock;
}

void TestAssistants::cleanupTestCase()
{
    Core::self()->cleanup();
    delete globalTestLock;
    globalTestLock = nullptr;
}

static QUrl createFile(const QString& fileContents, const QString& extension, int id)
{
    static QTemporaryDir tempDirA;
    Q_ASSERT(tempDirA.isValid());
    static QDir dirA(tempDirA.path());
    QFile file(dirA.filePath(QString::number(id) + extension));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(fileContents.toUtf8());
    file.close();
    return QUrl::fromLocalFile(file.fileName());
}

class Testbed
{
public:
    enum TestDoc
    {
        HeaderDoc,
        CppDoc
    };

    enum IncludeBehavior
    {
        NoAutoInclude,
        AutoInclude,
    };

    Testbed(const QString& headerContents, const QString& cppContents, IncludeBehavior include = AutoInclude)
        : m_includeBehavior(include)
    {
        static int i = 0;
        int id = i;
        ++i;
        m_headerDocument.url = createFile(headerContents,QStringLiteral(".h"),id);
        m_headerDocument.textDoc = openDocument(m_headerDocument.url);

        QString preamble;
        if (include == AutoInclude)
            preamble = QStringLiteral("#include \"%1\"\n").arg(m_headerDocument.url.toLocalFile());
        m_cppDocument.url = createFile(preamble + cppContents,QStringLiteral(".cpp"),id);
        m_cppDocument.textDoc = openDocument(m_cppDocument.url);
    }
    ~Testbed()
    {
        Core::self()->documentController()->documentForUrl(m_cppDocument.url)->textDocument();
        Core::self()->documentController()->documentForUrl(m_cppDocument.url)->close(KDevelop::IDocument::Discard);
        Core::self()->documentController()->documentForUrl(m_headerDocument.url)->close(KDevelop::IDocument::Discard);
    }

    void changeDocument(TestDoc which, Range where, const QString& what, bool waitForUpdate = false)
    {
        TestDocument document;
        if (which == CppDoc)
        {
            document = m_cppDocument;
            if (m_includeBehavior == AutoInclude) {
                where = Range(where.start().line() + 1, where.start().column(),
                              where.end().line() + 1, where.end().column()); //The include adds a line
            }
        }
        else {
            document = m_headerDocument;
        }
        // we must activate the document, otherwise we cannot find the correct active view
        auto kdevdoc = ICore::self()->documentController()->documentForUrl(document.url);
        QVERIFY(kdevdoc);
        ICore::self()->documentController()->activateDocument(kdevdoc);
        auto view = ICore::self()->documentController()->activeTextDocumentView();
        QCOMPARE(view->document(), document.textDoc);

        view->setSelection(where);
        view->removeSelectionText();
        view->setCursorPosition(where.start());
        view->insertText(what);
        QCoreApplication::processEvents();
        if (waitForUpdate) {
            DUChain::self()->waitForUpdate(IndexedString(document.url), KDevelop::TopDUContext::AllDeclarationsAndContexts);
        }
    }

    QString documentText(TestDoc which)
    {
        if (which == CppDoc)
        {
            //The CPP document text shouldn't include the autogenerated include line
            QString text = m_cppDocument.textDoc->text();
            return m_includeBehavior == AutoInclude ? text.mid(text.indexOf(QLatin1String("\n")) + 1) : text;
        }
        else
            return m_headerDocument.textDoc->text();
    }

    QString includeFileName() const
    {
        return m_headerDocument.url.toLocalFile();
    }

    KTextEditor::Document *document(TestDoc which) const
    {
        return Core::self()->documentController()->documentForUrl(
            which == CppDoc ? m_cppDocument.url : m_headerDocument.url)->textDocument();
    }

private:
    struct TestDocument {
        QUrl url;
        Document *textDoc;
    };

    Document* openDocument(const QUrl& url)
    {
        Core::self()->documentController()->openDocument(url);
        DUChain::self()->waitForUpdate(IndexedString(url), KDevelop::TopDUContext::AllDeclarationsAndContexts);
        return Core::self()->documentController()->documentForUrl(url)->textDocument();
    }

    IncludeBehavior m_includeBehavior;
    TestDocument m_headerDocument;
    TestDocument m_cppDocument;
};


/**
 * A StateChange describes an insertion/deletion/replacement and the expected result
**/
struct StateChange
{
    StateChange(){};
    StateChange(Testbed::TestDoc document, const Range& range, const QString& newText, const QString& result)
        : document(document)
        , range(range)
        , newText(newText)
        , result(result)
    {
    }
    Testbed::TestDoc document;
    Range range;
    QString newText;
    QString result;
};

Q_DECLARE_METATYPE(StateChange)

void TestAssistants::testRenameAssistant_data()
{
    QTest::addColumn<QString>("fileContents");
    QTest::addColumn<QString>("oldDeclarationName");
    QTest::addColumn<QList<StateChange> >("stateChanges");
    QTest::addColumn<QString>("finalFileContents");

    QTest::newRow("Prepend Text")
        << "int foo(int i)\n { i = 0; return i; }"
        << "i"
        << QList<StateChange>{
            StateChange(Testbed::CppDoc, Range(0,12,0,12), "u", "ui"),
            StateChange(Testbed::CppDoc, Range(0,13,0,13), "z", "uzi"),
        }
        << "int foo(int uzi)\n { uzi = 0; return uzi; }";

    QTest::newRow("Append Text")
        << "int foo(int i)\n { i = 0; return i; }"
        << "i"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0,13,0,13), QStringLiteral("d"), QStringLiteral("id")))
        << "int foo(int id)\n { id = 0; return id; }";

    QTest::newRow("Replace Text")
        << "int foo(int i)\n { i = 0; return i; }"
        << "i"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0,12,0,13), QStringLiteral("u"), QStringLiteral("u")))
        << "int foo(int u)\n { u = 0; return u; }";

    QTest::newRow("Paste Replace")
        << "int foo(int abg)\n { abg = 0; return abg; }"
        << "abg"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0,12,0,15), QStringLiteral("abcdefg"), QStringLiteral("abcdefg")))
        << "int foo(int abcdefg)\n { abcdefg = 0; return abcdefg; }";

    QTest::newRow("Paste Insert")
        << "int foo(int abg)\n { abg = 0; return abg; }"
        << "abg"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0,14,0,14), QStringLiteral("cdef"), QStringLiteral("abcdefg")))
        << "int foo(int abcdefg)\n { abcdefg = 0; return abcdefg; }";

    QTest::newRow("Letter-by-Letter Prepend")
        << "int foo(int i)\n { i = 0; return i; }"
        << "i"
        << (QList<StateChange>()
            << StateChange(Testbed::CppDoc, Range(0,12,0,12), QStringLiteral("a"), QStringLiteral("ai"))
            << StateChange(Testbed::CppDoc, Range(0,13,0,13), QStringLiteral("b"), QStringLiteral("abi"))
            << StateChange(Testbed::CppDoc, Range(0,14,0,14), QStringLiteral("c"), QStringLiteral("abci"))
        )
        << "int foo(int abci)\n { abci = 0; return abci; }";
    QTest::newRow("Letter-by-Letter Insert")
        << "int foo(int abg)\n { abg = 0; return abg; }"
        << "abg"
        << (QList<StateChange>()
            << StateChange(Testbed::CppDoc, Range(0,14,0,14), QStringLiteral("c"), QStringLiteral("abcg"))
            << StateChange(Testbed::CppDoc, Range(0,15,0,15), QStringLiteral("d"), QStringLiteral("abcdg"))
            << StateChange(Testbed::CppDoc, Range(0,16,0,16), QStringLiteral("e"), QStringLiteral("abcdeg"))
            << StateChange(Testbed::CppDoc, Range(0,17,0,17), QStringLiteral("f"), QStringLiteral("abcdefg"))
        )
        << "int foo(int abcdefg)\n { abcdefg = 0; return abcdefg; }";
}

ProblemPointer findStaticAssistantProblem(const QVector<ProblemPointer>& problems)
{
    const auto renameProblemIt = std::find_if(problems.cbegin(), problems.cend(), [](const ProblemPointer& p) {
        return dynamic_cast<const StaticAssistantProblem*>(p.constData());
    });
    if (renameProblemIt != problems.cend())
        return *renameProblemIt;

    return {};
}

template <typename T>
ProblemPointer findProblemWithAssistant(const QVector<ProblemPointer>& problems)
{
    const auto problemIterator = std::find_if(problems.cbegin(), problems.cend(), [](const ProblemPointer& p) {
        return dynamic_cast<const T*>(p->solutionAssistant().constData());
    });
    if (problemIterator != problems.cend())
        return *problemIterator;

    return {};
}

void TestAssistants::testRenameAssistant()
{
    QFETCH(QString, fileContents);
    Testbed testbed(QString(), fileContents);

    const auto document = testbed.document(Testbed::CppDoc);
    QVERIFY(document);

    QExplicitlySharedDataPointer<IAssistant> assistant;

    QFETCH(QString, oldDeclarationName);
    QFETCH(QList<StateChange>, stateChanges);
    for (const StateChange& stateChange : std::as_const(stateChanges)) {
        testbed.changeDocument(Testbed::CppDoc, stateChange.range, stateChange.newText, true);

        DUChainReadLocker lock;

        auto topCtx = DUChain::self()->chainForDocument(document->url());
        QVERIFY(topCtx);

        const auto problem = findStaticAssistantProblem(DUChainUtils::allProblemsForContext(topCtx));
        if (problem)
            assistant = problem->solutionAssistant();

        if (stateChange.result.isEmpty()) {
            QVERIFY(!assistant || !assistant->actions().size());
        } else {
            qWarning() << assistant.data() << stateChange.result;
            QVERIFY(assistant && assistant->actions().size());
            auto *r = qobject_cast<RenameAction*>(assistant->actions().first().data());
            QCOMPARE(r->oldDeclarationName(), oldDeclarationName);
            QCOMPARE(r->newDeclarationName(), stateChange.result);
        }
    }

    if (assistant && assistant->actions().size()) {
        assistant->actions().first()->execute();
    }
    QFETCH(QString, finalFileContents);
    QCOMPARE(testbed.documentText(Testbed::CppDoc), finalFileContents);
}

void TestAssistants::testRenameAssistantUndoRename()
{
    Testbed testbed(QString(), QStringLiteral("int foo(int i)\n { i = 0; return i; }"));
    testbed.changeDocument(Testbed::CppDoc, Range(0,13,0,13), QStringLiteral("d"), true);

    const auto document = testbed.document(Testbed::CppDoc);
    QVERIFY(document);

    DUChainReadLocker lock;
    auto topCtx = DUChain::self()->chainForDocument(document->url());
    QVERIFY(topCtx);

    auto firstProblem = findStaticAssistantProblem(DUChainUtils::allProblemsForContext(topCtx));
    QVERIFY(firstProblem);
    auto assistant = firstProblem->solutionAssistant();
    QVERIFY(assistant);

    QVERIFY(assistant->actions().size() > 0);
    auto *r = qobject_cast<RenameAction*>(assistant->actions().first().data());
    qWarning() << topCtx->problems() << assistant->actions().first().data() << assistant->actions().size();
    QVERIFY(r);

    // now rename the variable back to its original identifier
    testbed.changeDocument(Testbed::CppDoc, Range(0,13,0,14), QString());
    // there should be no assistant anymore
    QVERIFY(!assistant || assistant->actions().isEmpty());
}

const QString SHOULD_ASSIST = QStringLiteral("SHOULD_ASSIST"); //An assistant will be visible
const QString NO_ASSIST = QStringLiteral("NO_ASSIST");               //No assistant visible

void TestAssistants::testSignatureAssistant_data()
{
    QTest::addColumn<QString>("headerContents");
    QTest::addColumn<QString>("cppContents");
    QTest::addColumn<QList<StateChange> >("stateChanges");
    QTest::addColumn<QString>("finalHeaderContents");
    QTest::addColumn<QString>("finalCppContents");

    QTest::newRow("change_argument_type")
      << "class Foo {\nint bar(int a, char* b, int c = 10); \n};"
      << "int Foo::bar(int a, char* b, int c)\n{ a = c; b = new char; return a + *b; }"
      << (QList<StateChange>() << StateChange(Testbed::HeaderDoc, Range(1,8,1,11), QStringLiteral("char"), SHOULD_ASSIST))
      << "class Foo {\nint bar(char a, char* b, int c = 10); \n};"
      << "int Foo::bar(char a, char* b, int c)\n{ a = c; b = new char; return a + *b; }";

    QTest::newRow("prepend_arg_header")
      << "class Foo { void bar(int i); };"
      << "void Foo::bar(int i)\n{}"
      << (QList<StateChange>() << StateChange(Testbed::HeaderDoc, Range(0, 21, 0, 21), QStringLiteral("char c, "), SHOULD_ASSIST))
      << "class Foo { void bar(char c, int i); };"
      << "void Foo::bar(char c, int i)\n{}";

    QTest::newRow("prepend_arg_cpp")
      << "class Foo { void bar(int i); };"
      << "void Foo::bar(int i)\n{}"
      << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0, 14, 0, 14), QStringLiteral("char c, "), SHOULD_ASSIST))
      << "class Foo { void bar(char c, int i); };"
      << "void Foo::bar(char c, int i)\n{}";

    QTest::newRow("change_default_parameter")
        << "class Foo {\nint bar(int a, char* b, int c = 10); \n};"
        << "int Foo::bar(int a, char* b, int c)\n{ a = c; b = new char; return a + *b; }"
        << (QList<StateChange>() << StateChange(Testbed::HeaderDoc, Range(1,29,1,34), QString(), NO_ASSIST))
        << "class Foo {\nint bar(int a, char* b, int c); \n};"
        << "int Foo::bar(int a, char* b, int c)\n{ a = c; b = new char; return a + *b; }";

    QTest::newRow("change_function_type")
        << "class Foo {\nint bar(int a, char* b, int c = 10); \n};"
        << "int Foo::bar(int a, char* b, int c)\n{ a = c; b = new char; return a + *b; }"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0,0,0,3), QStringLiteral("char"), SHOULD_ASSIST))
        << "class Foo {\nchar bar(int a, char* b, int c = 10); \n};"
        << "char Foo::bar(int a, char* b, int c)\n{ a = c; b = new char; return a + *b; }";

    QTest::newRow("swap_args_definition_side")
        << "class Foo {\nint bar(int a, char* b, int c = 10); \n};"
        << "int Foo::bar(int a, char* b, int c)\n{ a = c; b = new char; return a + *b; }"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0,13,0,28), QStringLiteral("char* b, int a,"), SHOULD_ASSIST))
        << "class Foo {\nint bar(char* b, int a, int c = 10); \n};"
        << "int Foo::bar(char* b, int a, int c)\n{ a = c; b = new char; return a + *b; }";

    // see https://bugs.kde.org/show_bug.cgi?id=299393
    // actually related to the whitespaces in the header...
    QTest::newRow("change_function_constness")
        << "class Foo {\nvoid bar(const Foo&) const;\n};"
        << "void Foo::bar(const Foo&) const\n{}"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0,25,0,31), QString(), SHOULD_ASSIST))
        << "class Foo {\nvoid bar(const Foo&);\n};"
        << "void Foo::bar(const Foo&)\n{}";

    // see https://bugs.kde.org/show_bug.cgi?id=356179
    QTest::newRow("keep_static_cpp")
        << "class Foo { static void bar(int i); };"
        << "void Foo::bar(int i)\n{}"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0, 19, 0, 19), QStringLiteral(", char c"), SHOULD_ASSIST))
        << "class Foo { static void bar(int i, char c); };"
        << "void Foo::bar(int i, char c)\n{}";
    QTest::newRow("keep_static_header")
        << "class Foo { static void bar(int i); };"
        << "void Foo::bar(int i)\n{}"
        << (QList<StateChange>() << StateChange(Testbed::HeaderDoc, Range(0, 33, 0, 33), QStringLiteral(", char c"), SHOULD_ASSIST))
        << "class Foo { static void bar(int i, char c); };"
        << "void Foo::bar(int i, char c)\n{}";

    // see https://bugs.kde.org/show_bug.cgi?id=356178
    QTest::newRow("keep_default_args_cpp_before")
        << "class Foo { void bar(bool b, int i = 0); };"
        << "void Foo::bar(bool b, int i)\n{}"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0, 14, 0, 14), QStringLiteral("char c, "), SHOULD_ASSIST))
        << "class Foo { void bar(char c, bool b, int i = 0); };"
        << "void Foo::bar(char c, bool b, int i)\n{}";
    QTest::newRow("keep_default_args_cpp_after")
        << "class Foo { void bar(bool b, int i = 0); };"
        << "void Foo::bar(bool b, int i)\n{}"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0, 27, 0, 27), QStringLiteral(", char c"), SHOULD_ASSIST))
        << "class Foo { void bar(bool b, int i = 0, char c = {} /* TODO */); };"
        << "void Foo::bar(bool b, int i, char c)\n{}";
    QTest::newRow("keep_default_args_header_before")
        << "class Foo { void bar(bool b, int i = 0); };"
        << "void Foo::bar(bool b, int i)\n{}"
        << (QList<StateChange>() << StateChange(Testbed::HeaderDoc, Range(0, 29, 0, 29), QStringLiteral("char c = 'A', "), SHOULD_ASSIST))
        << "class Foo { void bar(bool b, char c = 'A', int i = 0); };"
        << "void Foo::bar(bool b, char c, int i)\n{}";
    QTest::newRow("keep_default_args_header_after")
        << "class Foo { void bar(bool b, int i = 0); };"
        << "void Foo::bar(bool b, int i)\n{}"
        << (QList<StateChange>() << StateChange(Testbed::HeaderDoc, Range(0, 38, 0, 38), QStringLiteral(", char c = 'A'"), SHOULD_ASSIST))
        << "class Foo { void bar(bool b, int i = 0, char c = 'A'); };"
        << "void Foo::bar(bool b, int i, char c)\n{}";

    // see https://bugs.kde.org/show_bug.cgi?id=355356
    QTest::newRow("no_retval_on_ctor")
        << "class Foo { Foo(); };"
        << "Foo::Foo()\n{}"
        << (QList<StateChange>() << StateChange(Testbed::HeaderDoc, Range(0, 16, 0, 16), QStringLiteral("char c"), SHOULD_ASSIST))
        << "class Foo { Foo(char c); };"
        << "Foo::Foo(char c)\n{}";

    // see https://bugs.kde.org/show_bug.cgi?id=365420
    QTest::newRow("no_retval_on_ctor_while_editing_definition")
        << "class Foo {\nFoo(int a); \n};"
        << "Foo::Foo(int a)\n{}"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0,13,0,14), QStringLiteral("b"), SHOULD_ASSIST))
        << "class Foo {\nFoo(int b); \n};"
        << "Foo::Foo(int b)\n{}";

    // see https://bugs.kde.org/show_bug.cgi?id=298511
    QTest::newRow("change_return_type_header")
        << "struct Foo { int bar(); };"
        << "int Foo::bar()\n{}"
        << (QList<StateChange>() << StateChange(Testbed::HeaderDoc, Range(0, 13, 0, 16), QStringLiteral("char"), SHOULD_ASSIST))
        << "struct Foo { char bar(); };"
        << "char Foo::bar()\n{}";
    QTest::newRow("change_return_type_impl")
        << "struct Foo { int bar(); };"
        << "int Foo::bar()\n{}"
        << (QList<StateChange>() << StateChange(Testbed::CppDoc, Range(0, 0, 0, 3), QStringLiteral("char"), SHOULD_ASSIST))
        << "struct Foo { char bar(); };"
        << "char Foo::bar()\n{}";
}

void TestAssistants::testSignatureAssistant()
{
    QFETCH(QString, headerContents);
    QFETCH(QString, cppContents);
    Testbed testbed(headerContents, cppContents);

    QExplicitlySharedDataPointer<IAssistant> assistant;

    QFETCH(QList<StateChange>, stateChanges);
    for (const StateChange& stateChange : std::as_const(stateChanges)) {
        testbed.changeDocument(stateChange.document, stateChange.range, stateChange.newText, true);

        const auto document = testbed.document(stateChange.document);
        QVERIFY(document);

        DUChainReadLocker lock;

        auto topCtx = DUChain::self()->chainForDocument(document->url());
        QVERIFY(topCtx);

        const auto problem = findProblemWithAssistant<AdaptSignatureAssistant>(DUChainUtils::allProblemsForContext(topCtx));
        if (problem) {
            assistant = problem->solutionAssistant();
        }

        if (stateChange.result == SHOULD_ASSIST) {
#if CINDEX_VERSION_MINOR < 35
            QEXPECT_FAIL("change_function_type", "Clang sees that return type of out-of-line definition differs from that in the declaration and won't parse the code...", Abort);
            QEXPECT_FAIL("change_return_type_impl", "Clang sees that return type of out-of-line definition differs from that in the declaration and won't include the function's AST and thus we never get updated about the new return type...", Abort);
#endif
            QVERIFY(assistant && !assistant->actions().isEmpty());
        } else {
            QVERIFY(!assistant || assistant->actions().isEmpty());
        }
    }

    if (assistant && !assistant->actions().isEmpty())
        assistant->actions().first()->execute();

    QFETCH(QString, finalHeaderContents);
    QFETCH(QString, finalCppContents);
    QCOMPARE(testbed.documentText(Testbed::HeaderDoc), finalHeaderContents);
    QCOMPARE(testbed.documentText(Testbed::CppDoc), finalCppContents);
}

enum UnknownDeclarationAction
{
    NoUnknownDeclarationAction = 0x0,
    ForwardDecls = 0x1,
    MissingInclude = 0x2
};
Q_DECLARE_FLAGS(UnknownDeclarationActions, UnknownDeclarationAction)
Q_DECLARE_METATYPE(UnknownDeclarationActions)

void TestAssistants::testUnknownDeclarationAssistant_data()
{
    QTest::addColumn<QString>("headerContents");
    QTest::addColumn<QString>("globalText");
    QTest::addColumn<QString>("functionText");
    QTest::addColumn<UnknownDeclarationActions>("actions");

    QTest::newRow("unincluded_struct") << "struct test{};" << "" << "test"
        << UnknownDeclarationActions(ForwardDecls | MissingInclude);
    QTest::newRow("forward_declared_struct") << "struct test{};" << "struct test;" << "test *f; f->"
        << UnknownDeclarationActions(MissingInclude);
    QTest::newRow("unknown_struct") << "" << "" << "test"
        << UnknownDeclarationActions();
    QTest::newRow("not a class type") << "void test();" << "" << "test"
        << UnknownDeclarationActions();
}

void TestAssistants::testUnknownDeclarationAssistant()
{
    QFETCH(QString, headerContents);
    QFETCH(QString, globalText);
    QFETCH(QString, functionText);
    QFETCH(UnknownDeclarationActions, actions);

    static const auto cppContents = QStringLiteral("%1\nvoid f_u_n_c_t_i_o_n() {\n}");
    Testbed testbed(headerContents, cppContents.arg(globalText), Testbed::NoAutoInclude);
    const auto document = testbed.document(Testbed::CppDoc);
    QVERIFY(document);
    const int line = document->lines() - 1;
    testbed.changeDocument(Testbed::CppDoc, Range(line, 0, line, 0), functionText, true);

    DUChainReadLocker lock;

    auto topCtx = DUChain::self()->chainForDocument(document->url());
    QVERIFY(topCtx);

    const auto problems = topCtx->problems();

    if (actions == NoUnknownDeclarationAction) {
        QVERIFY(!problems.isEmpty());
        return;
    }

    auto firstProblem = problems.first();
    auto assistant = firstProblem->solutionAssistant();
    QVERIFY(assistant);
    const auto assistantActions = assistant->actions();
    QStringList actionDescriptions;
    for (auto action: assistantActions) {
        actionDescriptions << action->description();
    }

    {
        const bool hasForwardDecls =
            actionDescriptions.contains(i18n("Forward declare as 'struct'")) ||
            actionDescriptions.contains(i18n("Forward declare as 'class'"));
        QCOMPARE(hasForwardDecls, static_cast<bool>(actions & ForwardDecls));
    }

    {
        auto fileName = testbed.includeFileName();
        fileName.remove(0, fileName.lastIndexOf('/') + 1);
        const auto directive = QStringLiteral("#include \"%1\"").arg(fileName);
        const auto description = i18n("Insert \'%1\'", directive);
        const bool hasMissingInclude = actionDescriptions.contains(description);
        QCOMPARE(hasMissingInclude, static_cast<bool>(actions & MissingInclude));
    }
}

void TestAssistants::testMoveIntoSource()
{
    QFETCH(QString, origHeader);
    QFETCH(QString, origImpl);
    QFETCH(QString, newHeader);
    QFETCH(QString, newImpl);
    QFETCH(QualifiedIdentifier, id);

    TestFile header(origHeader, QStringLiteral("h"));
    TestFile impl("#include \"" + header.url().byteArray() + "\"\n" + origImpl, QStringLiteral("cpp"), &header);

    {
        TopDUContext* headerCtx = nullptr;
        {
            DUChainReadLocker lock;
            headerCtx = DUChain::self()->chainForDocument(header.url());
        }
        // Here is a problem: when launching tests one by one, we can reuse the same tmp file for headers.
        // But because of document chain for header wasn't unloaded properly in previous run we reuse it here
        // Therefore when using headerCtx->findDeclarations below we find declarations from the previous launch -> tests fail
        if (headerCtx) {
            // TODO: Investigate why this chain doesn't get updated when parsing source file
            DUChainWriteLocker lock;
            DUChain::self()->removeDocumentChain(headerCtx);
        }
    }

    impl.parse(KDevelop::TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(impl.waitForParsed());

    IndexedDeclaration declaration;
    {
        DUChainReadLocker lock;
        auto headerCtx = DUChain::self()->chainForDocument(header.url());
        QVERIFY(headerCtx);
        auto decls = headerCtx->findDeclarations(id);
        Q_ASSERT(!decls.isEmpty());
        declaration = IndexedDeclaration(decls.first());
        QVERIFY(declaration.isValid());
    }
    CodeRepresentation::setDiskChangesForbidden(false);
    ClangRefactoring refactoring;
    QCOMPARE(refactoring.moveIntoSource(declaration), QString());
    CodeRepresentation::setDiskChangesForbidden(true);

    QCOMPARE(header.fileContents(), newHeader);
    QVERIFY(impl.fileContents().endsWith(newImpl));
}

void TestAssistants::testMoveIntoSource_data()
{
    QTest::addColumn<QString>("origHeader");
    QTest::addColumn<QString>("origImpl");
    QTest::addColumn<QString>("newHeader");
    QTest::addColumn<QString>("newImpl");
    QTest::addColumn<QualifiedIdentifier>("id");

    const QualifiedIdentifier fooId(QStringLiteral("foo"));

    QTest::newRow("globalfunction") << QStringLiteral("int foo()\n{\n    int i = 0;\n    return 0;\n}\n")
                                    << QString()
                                    << QStringLiteral("int foo();\n")
                                    << QStringLiteral("\nint foo()\n{\n    int i = 0;\n    return 0;\n}\n")
                                    << fooId;

    QTest::newRow("staticfunction") << QStringLiteral("static int foo()\n{\n    int i = 0;\n    return 0;\n}\n")
                                    << QString()
                                    << QStringLiteral("static int foo();\n")
                                    << QStringLiteral("\nint foo()\n{\n    int i = 0;\n    return 0;\n}\n")
                                    << fooId;

    QTest::newRow("funcsameline") << QStringLiteral("int foo() {\n    int i = 0;\n    return 0;\n}\n")
                                    << QString()
                                    << QStringLiteral("int foo();\n")
                                    << QStringLiteral("\nint foo() {\n    int i = 0;\n    return 0;\n}\n")
                                    << fooId;

    QTest::newRow("func-comment") << QStringLiteral("int foo()\n/* foobar */ {\n    int i = 0;\n    return 0;\n}\n")
                                    << QString()
                                    << QStringLiteral("int foo()\n/* foobar */;\n")
                                    << QStringLiteral("\nint foo() {\n    int i = 0;\n    return 0;\n}\n")
                                    << fooId;

    QTest::newRow("func-comment2") << QStringLiteral("int foo()\n/*asdf*/\n{\n    int i = 0;\n    return 0;\n}\n")
                                    << QString()
                                    << QStringLiteral("int foo()\n/*asdf*/;\n")
                                    << QStringLiteral("\nint foo()\n{\n    int i = 0;\n    return 0;\n}\n")
                                    << fooId;

    const QualifiedIdentifier aFooId(QStringLiteral("a::foo"));
    QTest::newRow("class-method") << QStringLiteral("class a {\n    int foo(){\n        return 0;\n    }\n};\n")
                                    << QString()
                                    << QStringLiteral("class a {\n    int foo();\n};\n")
                                    << QStringLiteral("\nint a::foo() {\n        return 0;\n    }\n")
                                    << aFooId;

    QTest::newRow("class-method-const") << QStringLiteral("class a {\n    int foo() const\n    {\n        return 0;\n    }\n};\n")
                                    << QString()
                                    << QStringLiteral("class a {\n    int foo() const;\n};\n")
                                    << QStringLiteral("\nint a::foo() const\n    {\n        return 0;\n    }\n")
                                    << aFooId;

    QTest::newRow("class-method-const-sameline") << QStringLiteral("class a {\n    int foo() const{\n        return 0;\n    }\n};\n")
                                    << QString()
                                    << QStringLiteral("class a {\n    int foo() const;\n};\n")
                                    << QStringLiteral("\nint a::foo() const {\n        return 0;\n    }\n")
                                    << aFooId;
    QTest::newRow("elaborated-type") << QStringLiteral("namespace NS{class C{};} class a {\nint foo(const NS::C c) const{\nreturn 0;\n}\n};\n")
                                    << QString()
                                    << QStringLiteral("namespace NS{class C{};} class a {\nint foo(const NS::C c) const;\n};\n")
                                    << QStringLiteral("\nint a::foo(const NS::C c) const {\nreturn 0;\n}\n")
                                    << aFooId;
    QTest::newRow("add-into-namespace") << QStringLiteral("namespace NS{class a {\nint foo() const {\nreturn 0;\n}\n};\n}")
                                    << QStringLiteral("namespace NS{\n}")
                                    << QStringLiteral("namespace NS{class a {\nint foo() const;\n};\n}")
                                    << QStringLiteral("namespace NS{\n\nint a::foo() const {\nreturn 0;\n}\n}")
                                    << QualifiedIdentifier(QStringLiteral("NS::a::foo"));
    QTest::newRow("class-template-parameter")
        << QStringLiteral(R"(
            namespace first {
            template <typename T>
            class Test{};

            namespace second {
                template <typename T>
                class List;
            }

            class MoveIntoSource
            {
            public:
                void f(const second::List<const volatile Test<first::second::List<int*>>*>& param){}
            };}
        )")
        << QString()
        << QStringLiteral(R"(
            namespace first {
            template <typename T>
            class Test{};

            namespace second {
                template <typename T>
                class List;
            }

            class MoveIntoSource
            {
            public:
                void f(const second::List<const volatile Test<first::second::List<int*>>*>& param);
            };}
        )")
        << QStringLiteral("namespace first {\nvoid MoveIntoSource::f(const first::second::List< const volatile first::Test< first::second::List< int* > >* >& param) {}}\n\n")
        << QualifiedIdentifier(QStringLiteral("first::MoveIntoSource::f"));

        QTest::newRow("move-unexposed-type")
            << QStringLiteral("namespace std { template<typename _CharT> class basic_string; \ntypedef basic_string<char> string;}\n void move(std::string i){}")
            << QString()
            << QStringLiteral("namespace std { template<typename _CharT> class basic_string; \ntypedef basic_string<char> string;}\n void move(std::string i);")
            << QStringLiteral("void move(std::string i) {}\n")
            << QualifiedIdentifier(QStringLiteral("move"));
        QTest::newRow("move-constructor")
            << QStringLiteral("class Class{Class(){}\n};")
            << QString()
            << QStringLiteral("class Class{Class();\n};")
            << QStringLiteral("Class::Class() {}\n")
            << QualifiedIdentifier(QStringLiteral("Class::Class"));
}

void TestAssistants::testHeaderGuardAssistant()
{
    CodeRepresentation::setDiskChangesForbidden(false);

    QFETCH(QString, filename);
    QFETCH(QString, code);
    QFETCH(QString, pragmaExpected);
    QFETCH(QString, macroExpected);

    TestFile pragmaFile(code, QStringLiteral("h"));
    TestFile macroFile(code, QStringLiteral("h"), filename);
    TestFile impl("#include \"" + pragmaFile.url().str() + "\"\n"
                  "#include \"" + macroFile.url().str() + "\"\n", QStringLiteral("cpp"));

    QExplicitlySharedDataPointer<IAssistant> pragmaAssistant;
    QExplicitlySharedDataPointer<IAssistant> macroAssistant;

    QVERIFY(impl.parseAndWait(TopDUContext::Empty));

    DUChainReadLocker lock;
    QVERIFY(impl.topContext());

    const auto pragmaTopContext = DUChain::self()->chainForDocument(pragmaFile.url());
    const auto macroTopContext = DUChain::self()->chainForDocument(macroFile.url());
    QVERIFY(pragmaTopContext);
    QVERIFY(macroTopContext);

    const auto pragmaProblem = findStaticAssistantProblem(DUChainUtils::allProblemsForContext(pragmaTopContext));
    const auto macroProblem = findStaticAssistantProblem(DUChainUtils::allProblemsForContext(macroTopContext));
    QVERIFY(pragmaProblem && macroProblem);
    pragmaAssistant = pragmaProblem->solutionAssistant();
    macroAssistant = macroProblem->solutionAssistant();
    QVERIFY(pragmaAssistant && macroAssistant);

    pragmaAssistant->actions()[0]->execute();
    macroAssistant->actions()[1]->execute();

    QCOMPARE(pragmaFile.fileContents(), pragmaExpected);
    QCOMPARE(macroFile.fileContents(), macroExpected);

    CodeRepresentation::setDiskChangesForbidden(true);
}

void TestAssistants::testHeaderGuardAssistant_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("code");
    QTest::addColumn<QString>("pragmaExpected");
    QTest::addColumn<QString>("macroExpected");

    QTest::newRow("simple") << QStringLiteral("simpleheaderguard")
        << QStringLiteral("int main()\n{\nreturn 0;\n}\n")
        << QStringLiteral("#pragma once\n\nint main()\n{\nreturn 0;\n}\n")
        << QStringLiteral(
            "#ifndef SIMPLEHEADERGUARD_H_INCLUDED\n"
            "#define SIMPLEHEADERGUARD_H_INCLUDED\n\n"
            "int main()\n{\nreturn 0;\n}\n\n"
            "#endif // SIMPLEHEADERGUARD_H_INCLUDED"
        );

    QTest::newRow("licensed") << QStringLiteral("licensed-headerguard")
        << QStringLiteral("/* Copyright 3019 John Doe\n */\n// Some comment\n"
                          "int main()\n{\nreturn 0;\n}\n")
        << QStringLiteral("/* Copyright 3019 John Doe\n */\n// Some comment\n"
                          "#pragma once\n\n"
                          "int main()\n{\nreturn 0;\n}\n")
        << QStringLiteral(
            "/* Copyright 3019 John Doe\n */\n// Some comment\n"
            "#ifndef LICENSED_HEADERGUARD_H_INCLUDED\n"
            "#define LICENSED_HEADERGUARD_H_INCLUDED\n\n"
            "int main()\n{\nreturn 0;\n}\n\n"
            "#endif // LICENSED_HEADERGUARD_H_INCLUDED"
        );

    QTest::newRow("empty") << QStringLiteral("empty-file")
        << QStringLiteral("")
        << QStringLiteral("#pragma once\n\n")
        << QStringLiteral("#ifndef EMPTY_FILE_H_INCLUDED\n"
                          "#define EMPTY_FILE_H_INCLUDED\n\n\n"
                          "#endif // EMPTY_FILE_H_INCLUDED"
        );

    QTest::newRow("no-trailinig-newline") << QStringLiteral("no-endline-file")
        << QStringLiteral("int foo;")
        << QStringLiteral("#pragma once\n\nint foo;")
        << QStringLiteral("#ifndef NO_ENDLINE_FILE_H_INCLUDED\n"
                            "#define NO_ENDLINE_FILE_H_INCLUDED\n\n"
                            "int foo;\n"
                            "#endif // NO_ENDLINE_FILE_H_INCLUDED"
        );

    QTest::newRow("whitespace-at-start") << QStringLiteral("whitespace-at-start")
        << QStringLiteral("\nint foo;")
        << QStringLiteral("#pragma once\n\n\nint foo;")
        << QStringLiteral(
            "#ifndef WHITESPACE_AT_START_H_INCLUDED\n"
            "#define WHITESPACE_AT_START_H_INCLUDED\n\n"
            "\nint foo;\n"
            "#endif // WHITESPACE_AT_START_H_INCLUDED"
        );
}

#include "moc_test_assistants.cpp"
