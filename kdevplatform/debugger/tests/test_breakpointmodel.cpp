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
#include <QMessageBox>
#include <QPoint>
#include <QScreen>
#include <QString>
#include <QTest>
#include <QUrl>

#include <algorithm>
#include <memory>
#include <vector>

QTEST_MAIN(TestBreakpointModel)

using namespace KDevelop;

namespace {

void showInstructionsForDialog(QMessageBox& instructions)
{
    // Remove the default button OK, because the user does not need to dismiss the instructions.
    instructions.setStandardButtons(QMessageBox::NoButton);

    // Show the message box before moving it, otherwise its screen() can be wrong.
    instructions.show();

    // The instructions message box appears in the center of a screen. The dialog, to which
    // the instructions apply, appears at the same position on top and hides the instructions.
    // Move the instructions message box down to ensure its visibility.
    const auto* const screen = instructions.screen();
    QVERIFY(screen);
    // On a multi-monitor X11 system, QScreen::availableSize() ignores window manager reserved areas
    // such as task bars and system menus, returns the same value as QScreen::size().
    // Reserve some space under the message box for possible task bars and panels.
    const int bottomY = screen->availableSize().height() * 3 / 4;
    instructions.move(instructions.pos().x(), bottomY - instructions.height());
}

/// Primary test file used as document in the temporary directory.
/// This file is always restored for a test to have same content.
constexpr const char* primaryTestFileName = "primary_test.cpp";

/// Secondary test file used as document in the temporary directory.
constexpr const char* secondaryTestFileName = "secondary_test.cpp";

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

/// Overwrite the contents of the local file at @p localFileUrl with @p newContents.
/// Check success with RETURN_IF_TEST_FAILED().
void overwriteExistingFile(const QUrl& localFileUrl, const QString& newContents)
{
    QFile file(localFileUrl.toLocalFile());
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::ExistingOnly));
    QVERIFY(file.write(newContents.toUtf8()) != -1);
    QVERIFY(file.flush());
}
} // unnamed namespace

/// Gather breakpoint marks in the document.
/// @return line numbers as int keys and BreakpointModel::MarkType (possibly a bitwise-or combination of) as mapped values.
TestBreakpointModel::DocumentMarks TestBreakpointModel::documentMarks(const IDocument* doc)
{
    DocumentMarks ret;
    // if it is not possible to get marks, fail.
    QVERIFY_RETURN(doc, ret);
    auto* const textDocument = doc->textDocument();
    QVERIFY_RETURN(textDocument, ret);

    const auto marks = textDocument->marks();
    for (const auto* mark : marks) {
        // mask to remove non-breakpoint mark type bits.
        const auto type = mark->type & BreakpointModel::AllBreakpointMarks;
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

/// Verify that the breakpoint is set correctly at the expected line number,
/// that the document line tracking is enabled for it, and that its mark type matches the expected value.
/// Check success with RETURN_IF_TEST_FAILED().
void TestBreakpointModel::verifyBreakpoint(Breakpoint* breakpoint, int expectedLine, uint expectedMarkType,
                                           const DocumentMarks& marks)
{
    QVERIFY(breakpoint->movingCursor());
    QCOMPARE(breakpoint->line(), expectedLine);

    // To be noted, there is no way to detect if an editor mark is actually
    // associated with a breakpoint instance. Because of this, the tests should
    // alternate using an enabled or disabled breakpoint to detect conflicts.
    QCOMPARE(breakpoint->markType(), expectedMarkType);
    const auto mark = marks.constFind(expectedLine);
    QVERIFY(mark != marks.cend());
    QCOMPARE(static_cast<uint>(mark.value()), expectedMarkType);
}

/// Convenience macro for verifyBreakpoint().
/// The fifth argument is an optional return value on failure.
#define VERIFY_BREAKPOINT(breakpoint, expectedLine, expectedMarkType, marks, ...)                                      \
    do {                                                                                                               \
        verifyBreakpoint(breakpoint, expectedLine, expectedMarkType, marks);                                           \
        RETURN_IF_TEST_FAILED(__VA_ARGS__);                                                                            \
    } while (false)

/// Verify that the breakpoint is set correctly at the expected line number,
/// and that the document line tracking is not enabled for it.
/// Check success with RETURN_IF_TEST_FAILED().
void TestBreakpointModel::verifyUntrackedBreakpoint(Breakpoint* breakpoint, int expectedLine)
{
    QVERIFY(!breakpoint->movingCursor());
    QCOMPARE(breakpoint->line(), expectedLine);
    QCOMPARE(breakpoint->savedLine(), expectedLine);
}

/// Convenience macro for verifyUntrackedBreakpoint().
/// The third argument is an optional return value on failure.
#define VERIFY_UNTRACKED_BREAKPOINT(breakpoint, expectedLine, ...)                                                     \
    do {                                                                                                               \
        verifyUntrackedBreakpoint(breakpoint, expectedLine);                                                           \
        RETURN_IF_TEST_FAILED(__VA_ARGS__);                                                                            \
    } while (false)

/// Verify that the breakpoint instances in the model are preserved.
/// Check success with RETURN_IF_TEST_FAILED().
void TestBreakpointModel::verifyTwoModelBreakpoints(Breakpoint* b1, Breakpoint* b2)
{
    QVERIFY(b1);
    QVERIFY(b2);

    QCOMPARE(breakpointModel()->rowCount(), 2);
    QVERIFY(breakpointModel()->breakpoint(0));
    QCOMPARE(breakpointModel()->breakpoint(0), b1);
    QVERIFY(breakpointModel()->breakpoint(1));
    QCOMPARE(breakpointModel()->breakpoint(1), b2);
}

void TestBreakpointModel::applyReloadModeAndReload(const QUrl& url, IDocument* doc, Breakpoint* b1)
{
    QFETCH(const ReloadMode, reloadMode);
    if (reloadMode != ReloadMode::Clean) {
        const auto* const textDoc = doc->textDocument();
        QString text;
        if (reloadMode == ReloadMode::DirtyBreakpointLine) {
            // Modify text on a breakpoint's line to prevent KTextEditor from restoring the breakpoint's mark when
            // the document is reloaded. KDevelop must ensure the breakpoint mark is restored in this case too.
            text = textDoc->text({{0, 0}, {b1->line(), 0}});
            text += textDoc->text({{b1->line(), 4}, textDoc->documentEnd()});
        } else {
            // Remove several lines from the end of the document's file.
            // This must not affect breakpoints above the removed lines.
            text = textDoc->text({{0, 0}, {textDoc->lines() - 3, 0}});
        }

        overwriteExistingFile(url, text);
        RETURN_IF_TEST_FAILED();
    }

    doc->reload();
}

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

    // Verify the initial state of a code breakpoint.
    auto* const b1 = breakpointModel()->breakpoint(0);
    QVERIFY_RETURN(b1, {});
    QCOMPARE_RETURN(b1->url(), url, {});
    QCOMPARE_RETURN(b1->kind(), Breakpoint::CodeBreakpoint, {});
    const auto marks = documentMarks(doc);
    QCOMPARE_RETURN(marks.size(), 1, {});
    VERIFY_BREAKPOINT(b1, 21, BreakpointModel::BreakpointMark, marks, {});
    QVERIFY_RETURN(b1->condition().isEmpty(), {});
    QCOMPARE_RETURN(b1->ignoreHits(), 0, {});
    QVERIFY_RETURN(b1->expression().isEmpty(), {});
    QCOMPARE_RETURN(b1->state(), Breakpoint::NotStartedState, {});
    QCOMPARE_RETURN(b1->hitCount(), 0, {});
    QCOMPARE_RETURN(b1->enabled(), true, {});

    return {url, doc, b1};
}

/// Prologue for tests that inserts two text lines and sets up two breakpoints in the primary document.
/// The second breakpoint is added below the first one on a moved text line.
/// Check success with RETURN_IF_TEST_FAILED().
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

    VERIFY_BREAKPOINT(b1, 23, BreakpointModel::DisabledBreakpointMark, marks, {});
    VERIFY_BREAKPOINT(b2, 24, BreakpointModel::BreakpointMark, marks, {});

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

    const auto marks = documentMarks(doc);
    QCOMPARE(marks.size(), 2);
    VERIFY_BREAKPOINT(b1, 22, BreakpointModel::BreakpointMark, marks, );
    VERIFY_BREAKPOINT(b2, 24, BreakpointModel::BreakpointMark, marks, );

    QVERIFY(doc->save());
    QVERIFY(doc->close());
    QCOMPARE(doc->state(), IDocument::Clean);

    // Wait needed for BreakpointModel::save() to complete.
    QTest::qWait(1);

    // verify.
    VERIFY_UNTRACKED_BREAKPOINT(b1, 22, );
    VERIFY_UNTRACKED_BREAKPOINT(b2, 24, );
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

    // save() shall make the breakpoint tracked line numbers persistent.
    // After saving, close() should have no effect on the line numbers
    // other than detaching all moving cursors from the document.
    QVERIFY(doc->save());
    QVERIFY(doc->close());
    QCOMPARE(doc->state(), IDocument::Clean);

    // Wait needed for BreakpointModel::save() to complete.
    QTest::qWait(1);

    // verify.
    VERIFY_UNTRACKED_BREAKPOINT(b1, 23, );
    VERIFY_UNTRACKED_BREAKPOINT(b2, 24, );
    const auto savedBreakpoints = readBreakpointsFromConfig();
    QCOMPARE(savedBreakpoints.size(), 2);
    QCOMPARE(savedBreakpoints.at(0)->line(), 23);
    QCOMPARE(savedBreakpoints.at(1)->line(), 24);
}

void TestBreakpointModel::testDocumentEditAndDiscard()
{
    const auto [url, doc, b1, b2] = setupEditAndCheckPrimaryDocumentAndBreakpoints();
    RETURN_IF_TEST_FAILED();

    // After discarding text changes b2->line() reverts to the line number 23.
    // This is where the text line tracking was started by addCodeBreakpoint() for b2.
    // Same applies to b1->line(), tracking which was started at the line 21 by openDocument().
    // Ideally b2->line() should be 22 after the discard, but this would be hard to implement.
    QVERIFY(doc->close(IDocument::DocumentSaveMode::Discard));

    // Wait needed for BreakpointModel::save() to complete.
    QTest::qWait(1);

    // verify.
    VERIFY_UNTRACKED_BREAKPOINT(b1, 21, );
    VERIFY_UNTRACKED_BREAKPOINT(b2, 23, );
    const auto savedBreakpoints = readBreakpointsFromConfig();
    QCOMPARE(savedBreakpoints.size(), 2);
    QCOMPARE(savedBreakpoints.at(0)->line(), 21);
    QCOMPARE(savedBreakpoints.at(1)->line(), 23);
}

void TestBreakpointModel::testSetLocation()
{
    const auto [url1, doc1, b1] = setupPrimaryDocumentAndBreakpoint();
    RETURN_IF_TEST_FAILED();

    // TEST: increment the line number.
    b1->setLocation(url1, b1->savedLine() + 1);
    // Verify the move of the mark and the change of location.
    QCOMPARE(breakpointModel()->breakpoint(0), b1);
    auto marks = documentMarks(doc1);
    QCOMPARE(marks.size(), 1);
    VERIFY_BREAKPOINT(b1, 22, BreakpointModel::BreakpointMark, marks, );

    // Make a secondary test file and open it.
    QVERIFY(QFile::copy(m_tempDir->filePath(primaryTestFileName), m_tempDir->filePath(secondaryTestFileName)));
    const auto url2 = testFileUrl(secondaryTestFileName);
    RETURN_IF_TEST_FAILED();
    auto* const doc2 = documentController()->openDocument(url2);
    QVERIFY(doc2);
    QCOMPARE(doc2->url(), url2);

    // TEST: Change b1 location to the secondary document.
    b1->setLocation(url2, 21);
    // Verify the move of the mark and the change of location.
    QCOMPARE(b1->url(), url2);
    QCOMPARE(breakpointModel()->breakpoint(0), b1);
    QCOMPARE(documentMarks(doc1).size(), 0);
    marks = documentMarks(doc2);
    QCOMPARE(marks.size(), 1);
    VERIFY_BREAKPOINT(b1, 21, BreakpointModel::BreakpointMark, marks, );

    // TEST: Make b1 fall out of the document bounds.
    //       This should disable b1's document line tracking.
    const auto lineNumberAfterLastLineInDocument = doc2->textDocument()->lines();
    b1->setLocation(url2, lineNumberAfterLastLineInDocument);
    // Verify the removal of the mark and the change of line number.
    VERIFY_UNTRACKED_BREAKPOINT(b1, lineNumberAfterLastLineInDocument, );
    QCOMPARE(documentMarks(doc1).size(), 0);
    QCOMPARE(documentMarks(doc2).size(), 0);
}

void TestBreakpointModel::testUpdateMarkType()
{
    const auto [url, doc, b1] = setupPrimaryDocumentAndBreakpoint();
    RETURN_IF_TEST_FAILED();

    // TEST: Change the mark type to pending.
    b1->setState(Breakpoint::PendingState);
    // Verify.
    QCOMPARE(b1->state(), Breakpoint::PendingState);
    VERIFY_BREAKPOINT(b1, 21, BreakpointModel::PendingBreakpointMark, documentMarks(doc), );

    // TEST: Change the mark type to reached.
    b1->setHitCount(1);
    // Verify.
    QCOMPARE(b1->hitCount(), 1);
    VERIFY_BREAKPOINT(b1, 21, BreakpointModel::ReachedBreakpointMark, documentMarks(doc), );

    // TEST: Change the mark type to disabled.
    b1->setData(Breakpoint::EnableColumn, Qt::Unchecked);
    // Verify.
    QCOMPARE(b1->enabled(), false);
    VERIFY_BREAKPOINT(b1, 21, BreakpointModel::DisabledBreakpointMark, documentMarks(doc), );
}

void TestBreakpointModel::testDocumentReload_data()
{
    QTest::addColumn<ReloadMode>("reloadMode");
    QTest::newRow("Clean") << ReloadMode::Clean;
    QTest::newRow("Dirty") << ReloadMode::Dirty;
    QTest::newRow("DirtyBreakpointLine") << ReloadMode::DirtyBreakpointLine;
}

void TestBreakpointModel::testDocumentReload()
{
    const auto [url, doc, b1, b2] = setupEditAndCheckPrimaryDocumentAndBreakpoints();
    RETURN_IF_TEST_FAILED();

    // Set some extra breakpoint data that should be preserved even after a reload.
    b1->setIgnoreHits(1);
    b1->setCondition("i == 0");
    b1->setHitCount(1);
    b2->setIgnoreHits(2);
    b2->setCondition("*i > 0");
    b2->setHitCount(2);

    QVERIFY(doc->save());
    QCOMPARE(doc->state(), IDocument::Clean);

    // Wait needed for BreakpointModel::save() to complete.
    QTest::qWait(1);

    applyReloadModeAndReload(url, doc, b1);
    RETURN_IF_TEST_FAILED();

    verifyTwoModelBreakpoints(b1, b2);
    RETURN_IF_TEST_FAILED();

    // Verify the breakpoints' locations and marks.
    const auto marks = documentMarks(doc);
    QCOMPARE(marks.size(), 2);
    VERIFY_BREAKPOINT(b1, 23, BreakpointModel::DisabledBreakpointMark, marks, );
    VERIFY_BREAKPOINT(b2, 24, BreakpointModel::ReachedBreakpointMark, marks, );

    // Verify that reloading does not affect unrelated breakpoint data.
    QCOMPARE(b1->enabled(), false);
    QCOMPARE(b1->ignoreHits(), 1);
    QCOMPARE(b1->condition(), "i == 0");
    QCOMPARE(b1->hitCount(), 1);
    QCOMPARE(b2->enabled(), true);
    QCOMPARE(b2->ignoreHits(), 2);
    QCOMPARE(b2->condition(), "*i > 0");
    QCOMPARE(b2->hitCount(), 2);

    const auto savedBreakpoints = readBreakpointsFromConfig();
    QCOMPARE(savedBreakpoints.size(), 2);
    QCOMPARE(savedBreakpoints.at(0)->line(), 23);
    QCOMPARE(savedBreakpoints.at(0)->enabled(), false);
    QCOMPARE(savedBreakpoints.at(0)->ignoreHits(), 1);
    QCOMPARE(savedBreakpoints.at(0)->condition(), "i == 0");
    QCOMPARE(savedBreakpoints.at(1)->line(), 24);
    QCOMPARE(savedBreakpoints.at(1)->enabled(), true);
    QCOMPARE(savedBreakpoints.at(1)->ignoreHits(), 2);
    QCOMPARE(savedBreakpoints.at(1)->condition(), "*i > 0");
}

void TestBreakpointModel::testModifiedDocumentReload_data()
{
    using UserChoice = CloseDocumentUserChoice;
    QTest::addColumn<ReloadMode>("reloadMode");
    QTest::addColumn<UserChoice>("userChoice");

    for (const auto reloadMode : {ReloadMode::Clean, ReloadMode::Dirty, ReloadMode::DirtyBreakpointLine}) {
        for (const auto userChoice : {UserChoice::Cancel, UserChoice::Discard, UserChoice::Save}) {
            QTest::addRow("%s-%s", enumeratorName(reloadMode), enumeratorName(userChoice)) << reloadMode << userChoice;
        }
    }
}

void TestBreakpointModel::testModifiedDocumentReload()
{
#if 1
    QSKIP(
        "This test is not fully automatic. In order to test breakpoints when a modified document is reloaded, "
        "disable this QSKIP() call and make choices specified by the Test Instructions dialog.");
#endif

    const auto [url, doc, b1, b2] = setupEditAndCheckPrimaryDocumentAndBreakpoints();
    RETURN_IF_TEST_FAILED();

    // Set some extra breakpoint data that should be preserved even after a reload.
    b1->setIgnoreHits(11);
    b2->setHitCount(20);

    QCOMPARE(doc->state(), IDocument::Modified);

    using UserChoice = CloseDocumentUserChoice;
    QFETCH(const UserChoice, userChoice);

    QMessageBox choiceInstruction(
        QMessageBox::Information, "Test Instructions",
        QLatin1String("Choose <b>%1</b> in the Close Document warning dialog.").arg(enumeratorName(userChoice)));
    showInstructionsForDialog(choiceInstruction);
    RETURN_IF_TEST_FAILED();

    // Testing different reload modes is important only in case of the Discard user choice, because otherwise
    // the document either is not reloaded from disk at all (Cancel) or saved over the changes on disk first (Save).
    // The impact of reload modes in case of the Discard user choice:
    // 1) Clean: two lines inserted over the two breakpoint lines in the edited document are absent from disk =>
    //    both breakpoint lines are changed by the reloading => KTextEditor does not restore any breakpoint marks.
    // 2) Dirty: applyReloadModeAndReload() saves the edited version of the document sans a few unimportant lines at the
    //    end => none of the breakpoint lines is changed by the reloading => KTextEditor restores both breakpoint marks.
    // 3) DirtyBreakpointLine: applyReloadModeAndReload() saves the edited version of the document but also modifies
    //    b1's line => b1's line is and b2's is not changed by the reloading => KTextEditor restores b2, but not b1.
    applyReloadModeAndReload(url, doc, b1);
    RETURN_IF_TEST_FAILED();

    // Wait needed for BreakpointModel::save() to complete.
    QTest::qWait(1);

    verifyTwoModelBreakpoints(b1, b2);
    RETURN_IF_TEST_FAILED();

    QCOMPARE(doc->state(), userChoice == UserChoice::Cancel ? IDocument::Modified : IDocument::Clean);

    struct
    {
        int b1 = 23, b2 = 24;
    } lines, savedLines;
    switch (userChoice) {
    case UserChoice::Discard:
        lines.b1 = 21;
        lines.b2 = 23;
        [[fallthrough]];
    case UserChoice::Cancel:
        savedLines.b1 = 21;
        savedLines.b2 = 23;
        break;
    case UserChoice::Save:
        break;
    }

    // Verify the breakpoints' locations and marks.
    const auto marks = documentMarks(doc);
    QCOMPARE(marks.size(), 2);
    VERIFY_BREAKPOINT(b1, lines.b1, BreakpointModel::DisabledBreakpointMark, marks, );
    VERIFY_BREAKPOINT(b2, lines.b2, BreakpointModel::ReachedBreakpointMark, marks, );

    // Verify that reloading does not affect unrelated breakpoint data.
    QCOMPARE(b1->enabled(), false);
    QCOMPARE(b1->ignoreHits(), 11);
    QCOMPARE(b2->enabled(), true);
    QCOMPARE(b2->hitCount(), 20);

    const auto savedBreakpoints = readBreakpointsFromConfig();
    QCOMPARE(savedBreakpoints.size(), 2);
    QCOMPARE(savedBreakpoints.at(0)->line(), savedLines.b1);
    QCOMPARE(savedBreakpoints.at(0)->enabled(), false);
    QCOMPARE(savedBreakpoints.at(0)->ignoreHits(), 11);
    QCOMPARE(savedBreakpoints.at(1)->line(), savedLines.b2);
    QCOMPARE(savedBreakpoints.at(1)->enabled(), true);
}

#include "moc_test_breakpointmodel.cpp"
