/*
 *  SPDX-FileCopyrightText: 2023 JATothrim <jarmo.tiitto@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "test_breakpointmodel.h"

#include "testfilepaths.h"

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/isession.h>
#include <language/backgroundparser/backgroundparser.h>
#include <shell/documentcontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KTextEditor/Cursor>
#include <KTextEditor/Document>

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTest>
#include <QUrl>

#include <algorithm>
#include <memory>
#include <vector>

QTEST_MAIN(TestBreakpointModel)

using namespace KDevelop;

/// Expected test helper failure handling.
/// Use QEXPECT_FAIL_ABORT() to mark the next check as an expected failure,
/// and call RETURN_IF_TEST_ABORTED() right after RETURN_IF_TEST_FAILED()
/// to check if the test helper aborted.

static constexpr const char* testAbortPropertyName = "kdevelop.test.abort";

#define QEXPECT_FAIL_ABORT(comment)                                                                                    \
    do {                                                                                                               \
        QTest::testObject()->setProperty(testAbortPropertyName, true);                                                 \
        QEXPECT_FAIL("", comment, Abort);                                                                              \
    } while (false)

#define RETURN_IF_TEST_ABORTED(...)                                                                                    \
    do {                                                                                                               \
        if (QTest::testObject()->property(testAbortPropertyName).isValid()) {                                          \
            qCritical("ABORTED AT: %s:%d", __FILE__, __LINE__);                                                        \
            return __VA_ARGS__;                                                                                        \
        }                                                                                                              \
    } while (false)

namespace {

/// Primary test file used as document in the temporary directory.
/// This file is always restored for a test to have same content.
constexpr const char* primaryTestFileName = "primary_test.cpp";

/// Get breakpoint config group.
/// NOTE: this config is written into
/// "~/.qttest/share/test_breakpointmodel/sessions/{session-UUID}/sessionrc"
KConfigGroup breakpointConfig()
{
    return ICore::self()->activeSession()->config()->group("Breakpoints");
}

/// Qt 5.15 does not have C++11 emplace() like functionality.
/// Make use of QList< std::shared_ptr< Breakpoint > > instead.
using BreakpointPtr = std::shared_ptr<Breakpoint>;

/// Read BreakpointModel config data, like BreakpointModel::load() does.
/// The Breakpoints are not attached to the model, so they stay as standalone instances.
QList<BreakpointPtr> readBreakpointsFromConfig()
{
    QList<BreakpointPtr> data;
    const KConfigGroup config = breakpointConfig();
    const int count = config.readEntry("number", 0);

    for (int i = 0; i < count; ++i) {
        const auto group = config.group(QString::number(i));
        QVERIFY_RETURN(!group.readEntry("kind", "").isEmpty(), {});
        data.push_back(std::make_shared<Breakpoint>(nullptr, group));
    }
    return data;
}

IDocumentController* documentController()
{
    return ICore::self()->documentController();
}

BreakpointModel* breakpointModel()
{
    return ICore::self()->debugController()->breakpointModel();
}

} // unnamed namespace

/// Gather breakpoint marks in the document.
/// @return line numbers as int keys and BreakpointModel::MarkType (possibly a bitwise-or combination of) as mapped values.
TestBreakpointModel::DocumentMarks TestBreakpointModel::documentMarks(const IDocument* doc)
{
    DocumentMarks ret;
    // if it is not possible to get marks, fail.
    QVERIFY_RETURN(doc, ret);
    auto* const imark = qobject_cast<KTextEditor::MarkInterface*>(doc->textDocument());
    QVERIFY_RETURN(imark, ret);

    const auto marks = imark->marks();
    for (const auto* mark : marks) {
        // mask to remove non-breakpoint mark type bits.
        const auto type = mark->type & BreakpointModel::MarkType::AllBreakpointMarks;
        if (type) {
            ret.insert(mark->line, static_cast<BreakpointModel::MarkType>(type));
        }
    }
    return ret;
}

/// Print sections of example code.
/// Used for checking what edited text looks like when writing tests.
void TestBreakpointModel::printLines(int from, int count, const IDocument* doc)
{
    // gather a set of breakpoint line numbers in doc.
    std::vector<int> breakpointLines;
    const auto breakpoints = breakpointModel()->breakpoints();
    for (const auto* b : breakpoints) {
        if (b->url() == doc->url()) {
            breakpointLines.push_back(b->line());
        }
    }
    std::sort(breakpointLines.begin(), breakpointLines.end());

    // gather a set of breakpoint marks in doc.
    const auto markLines = documentMarks(doc);

    // Breakpoint prefixes:
    // "[bm]" ok: a line has both mark and breakpoint instance
    // "[b-]" a line has breakpoint instance but no mark in UI
    // "[-m]" a line has mark in UI but no breakpoint instance
    // TODO: print upper-case letters for marks or breakpoints that are in an enabled state.
    for (int i = from; i < from + count && i < doc->textDocument()->lines(); ++i) {
        const bool hasBreakpoint = std::binary_search(breakpointLines.cbegin(), breakpointLines.cend(), i);
        QString prefix = (hasBreakpoint ? "b" : "-");
        prefix += (markLines.contains(i) ? "m" : "-");

        qDebug("%d\t[%s]: %s", i, qUtf8Printable(prefix), qUtf8Printable(doc->textDocument()->line(i)));
    }
}

/// Verify that the breakpoint is set correctly at the expected line number.
/// Check success with RETURN_IF_TEST_FAILED() and RETURN_IF_TEST_ABORTED().
void TestBreakpointModel::verifyBreakpoint(Breakpoint* breakpoint, int expectedLine, const DocumentMarks& marks,
                                           bool expectFail)
{
    if (expectFail) {
        QEXPECT_FAIL_ABORT("CodeBreakpoint tracked line number is not updated");
    }
    QCOMPARE(breakpoint->line(), expectedLine);

    // To be noted, there is no way to detect if an editor mark is actually
    // associated with a breakpoint instance. Because of this, the tests should
    // alternate using an enabled or disabled breakpoint to detect conflicts.
    const auto mark = marks.constFind(expectedLine);
    if (expectFail) {
        QEXPECT_FAIL_ABORT("CodeBreakpoint mark does not follow the tracking");
    }
    QVERIFY(mark != marks.cend());

    const auto breakpointType = breakpointModel()->breakpointType(breakpoint);
    QCOMPARE(static_cast<uint>(mark.value()), breakpointType);
}

/// Convenience macro for verifyBreakpoint().
/// The fourth argument is an optional return value on failure.
#define VERIFY_BREAKPOINT(breakpoint, expectedLine, marks, expectFail, ...)                                            \
    do {                                                                                                               \
        verifyBreakpoint(breakpoint, expectedLine, marks, expectFail);                                                 \
        RETURN_IF_TEST_FAILED(__VA_ARGS__);                                                                            \
        RETURN_IF_TEST_ABORTED(__VA_ARGS__);                                                                           \
    } while (false)

TestBreakpointModel::TestBreakpointModel(QObject* parent)
    : QObject(parent)
{
}

/// Get URL to existing test file under the temporary dir.
QUrl TestBreakpointModel::testFileUrl(const QString& fileName) const
{
    const QFileInfo info(m_tempDir->path(), fileName);
    QVERIFY_RETURN(info.isFile(), QUrl{});
    auto url = QUrl::fromLocalFile(info.canonicalFilePath());
    QVERIFY_RETURN(url.isValid(), QUrl{});
    return url;
}

/// Prologue for tests that use the primary test file and the breakpoint created in init().
/// Check success with RETURN_IF_TEST_FAILED().
TestBreakpointModel::DocumentAndBreakpoint TestBreakpointModel::setupPrimaryDocumentAndBreakpoint()
{
    // setup.
    breakpointModel()->load();
    QCOMPARE_RETURN(breakpointModel()->rowCount(), 1, {});

    const auto url = testFileUrl(primaryTestFileName);
    RETURN_IF_TEST_FAILED({});
    auto* const doc = documentController()->openDocument(url);
    QVERIFY_RETURN(doc, {});
    QCOMPARE_RETURN(doc->url(), url, {});

    // pre-conditions.
    auto* const b1 = breakpointModel()->breakpoint(0);
    QVERIFY_RETURN(b1, {});
    QCOMPARE_RETURN(b1->url(), url, {});
    QCOMPARE_RETURN(b1->line(), 21, {});
    QVERIFY_RETURN(b1->condition().isEmpty(), {});
    QCOMPARE_RETURN(b1->ignoreHits(), 0, {});
    QVERIFY_RETURN(b1->expression().isEmpty(), {});
    QVERIFY_RETURN(b1->movingCursor(), {});

    return {url, doc, b1};
}

/// Prologue for tests that inserts two text lines and sets up two breakpoints in the primary document.
/// The second breakpoint is added below the first one on a moved text line.
/// Check success with RETURN_IF_TEST_FAILED() and RETURN_IF_TEST_ABORTED().
TestBreakpointModel::DocumentAndTwoBreakpoints TestBreakpointModel::setupEditAndCheckPrimaryDocumentAndBreakpoints()
{
    const auto [url, doc, b1] = setupPrimaryDocumentAndBreakpoint();
    RETURN_IF_TEST_FAILED({});

    // set b1 as disabled, so its mark differs from b2 in the editor.
    b1->setData(Breakpoint::EnableColumn, Qt::Unchecked);

    // test.
    doc->textDocument()->insertLine(21, "// Comment A");
    auto* const b2 = breakpointModel()->addCodeBreakpoint(url, 23);
    doc->textDocument()->insertLine(21, "// Comment B");
    printLines(21, 4, doc);

    // saved line numbers shall not change until the document is saved.
    QCOMPARE_RETURN(b1->savedLine(), 21, {});
    QCOMPARE_RETURN(b2->savedLine(), 23, {});

    const auto marks = documentMarks(doc);
    QCOMPARE_RETURN(marks.size(), 2, {});

    VERIFY_BREAKPOINT(b1, 23, marks, false, {});
    VERIFY_BREAKPOINT(b2, 24, marks, true, {});

    return {url, doc, b1, b2};
}

void TestBreakpointModel::initTestCase()
{
    AutoTestShell::init({{}});
    TestCore::initialize();
    ICore::self()->languageController()->backgroundParser()->disableProcessing();

    // NOTE: DebugController is already initialized at this point,
    // and BreakpointModel::load() has run.
    // The model may have loaded some breakpoints from last run,
    // that must be cleared by init().
}

void TestBreakpointModel::init()
{
    // Reset the test abort property before each test function.
    QTest::testObject()->setProperty(testAbortPropertyName, QVariant{});

    Core::self()->documentControllerInternal()->initialize();

    // Restore the primary test file under empty temporary working dir.
    m_tempDir = std::make_unique<QTemporaryDir>();
    QVERIFY(QFile::copy(TEST_FILES_DIR "/primary_test.cpp", m_tempDir->filePath(primaryTestFileName)));

    // pre-conditions
    QVERIFY(documentController());
    QVERIFY(breakpointModel());
    QVERIFY(documentController()->openDocuments().empty());

    // init() must put breakpointModel() and breakpointConfig()
    // into state where "breakpointModel()->load()" can work:
    // - breakpointConfig() has single breakpoint.
    // - No breakpoints are registered/exist in the model.

    // Remove all breakpoints, overwrites breakpointConfig().
    // This must be done *before* writing the hard-coded config below.
    auto* const model = breakpointModel();
    model->removeRows(0, model->rowCount());

    // Setup hard-coded breakpoint model config with single breakpoint.
    // Tests can use this by doing "breakpointModel()->load()" before modifying the BreakpointModel.
    KConfigGroup config = breakpointConfig();
    config.writeEntry("number", 1);
    // (ab)use Breakpoint to write the config data for it.
    Breakpoint entry(nullptr, Breakpoint::CodeBreakpoint);
    // set source line location at "foo(5);"
    const auto url = testFileUrl(primaryTestFileName);
    RETURN_IF_TEST_FAILED();
    entry.setLocation(url, 21);
    {
        KConfigGroup group = config.group(QString::number(0));
        entry.save(group);
    }

    QCOMPARE(breakpointModel()->rowCount(), 0);
}

void TestBreakpointModel::cleanup()
{
    Core::self()->documentControllerInternal()->cleanup();
}

void TestBreakpointModel::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestBreakpointModel::testDocumentSave()
{
    const auto [url, doc, b1] = setupPrimaryDocumentAndBreakpoint();
    RETURN_IF_TEST_FAILED();

    // test.
    b1->setLocation(url, 22);
    auto* const b2 = breakpointModel()->addCodeBreakpoint(url, 24);
    b2->setCondition("*i > 0");
    b2->setIgnoreHits(1);
    printLines(21, 4, doc);

    QVERIFY(b1->movingCursor());

    // FIXME: all addCodeBreakpoint(url, line) should gain moving cursor.
    QEXPECT_FAIL("", "Added CodeBreakpoint b2 does not have a moving cursor", Continue);
    QVERIFY(b2->movingCursor());

    const auto marks = documentMarks(doc);
    QCOMPARE(marks.size(), 2);
    QVERIFY(marks.contains(b1->line()));
    QVERIFY(marks.contains(b2->line()));

    QVERIFY(doc->save());
    QVERIFY(doc->close());
    QCOMPARE(doc->state(), IDocument::Clean);

    // Wait needed for BreakpointModel::save() to complete.
    QTest::qWait(1);

    // verify.
    QVERIFY(!b1->movingCursor());
    QVERIFY(!b2->movingCursor());
    QCOMPARE(b1->savedLine(), 22);
    QCOMPARE(b2->savedLine(), 24);
    const auto savedBreakpoints = readBreakpointsFromConfig();
    QCOMPARE(savedBreakpoints.size(), 2);
    QCOMPARE(savedBreakpoints.at(0)->line(), 22);
    QCOMPARE(savedBreakpoints.at(1)->line(), 24);
    QCOMPARE(savedBreakpoints.at(1)->condition(), "*i > 0");
    QCOMPARE(savedBreakpoints.at(1)->ignoreHits(), 1);
}

void TestBreakpointModel::testDocumentEditAndSave()
{
    const auto [url, doc, b1, b2] = setupEditAndCheckPrimaryDocumentAndBreakpoints();
    RETURN_IF_TEST_FAILED();
    RETURN_IF_TEST_ABORTED();

    // save() shall make the breakpoint tracked line numbers persistent.
    // After saving, close() should have no effect on the line numbers
    // other than detaching all moving cursors from the document.
    QVERIFY(doc->save());
    QVERIFY(doc->close());
    QCOMPARE(doc->state(), IDocument::Clean);

    // Wait needed for BreakpointModel::save() to complete.
    QTest::qWait(1);

    // verify.
    QVERIFY(!b1->movingCursor());
    QVERIFY(!b2->movingCursor());
    QCOMPARE(b1->savedLine(), 23);
    QEXPECT_FAIL("", "CodeBreakpoint b2 tracked line number is not retained", Continue);
    QCOMPARE(b2->savedLine(), 24);
    const auto savedBreakpoints = readBreakpointsFromConfig();
    QCOMPARE(savedBreakpoints.size(), 2);
    QCOMPARE(savedBreakpoints.at(0)->line(), 23);
    QEXPECT_FAIL("", "CodeBreakpoint b2 tracked line number is not persistent", Continue);
    QCOMPARE(savedBreakpoints.at(1)->line(), 24);
}

void TestBreakpointModel::testDocumentEditAndDiscard()
{
    const auto [url, doc, b1, b2] = setupEditAndCheckPrimaryDocumentAndBreakpoints();
    RETURN_IF_TEST_FAILED();
    RETURN_IF_TEST_ABORTED();

    // After discarding text changes b2->line() reverts to the line number 23.
    // This is where the text line tracking was started by addCodeBreakpoint() for b2.
    // Same applies to b1->line(), tracking which was started at the line 21 by openDocument().
    // Ideally b2->line() should be 22 after the discard, but this would be hard to implement.
    QVERIFY(doc->close(IDocument::DocumentSaveMode::Discard));

    // Wait needed for BreakpointModel::save() to complete.
    QTest::qWait(1);

    // verify.
    QVERIFY(!b1->movingCursor());
    QVERIFY(!b2->movingCursor());
    QCOMPARE(b1->savedLine(), 21);
    QCOMPARE(b2->savedLine(), 23);
    const auto savedBreakpoints = readBreakpointsFromConfig();
    QCOMPARE(savedBreakpoints.size(), 2);
    QCOMPARE(savedBreakpoints.at(0)->line(), 21);
    QCOMPARE(savedBreakpoints.at(1)->line(), 23);
}

#include "moc_test_breakpointmodel.cpp"
