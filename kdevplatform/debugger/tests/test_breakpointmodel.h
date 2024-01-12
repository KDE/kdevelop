/*
 *  SPDX-FileCopyrightText: 2023 JATothrim <jarmo.tiitto@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef KDEVPLATFORM_TEST_BREAKPOINTMODEL_H
#define KDEVPLATFORM_TEST_BREAKPOINTMODEL_H

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <interfaces/idocument.h>

#include <QHash>
#include <QObject>
#include <QTemporaryDir>
#include <QUrl>

#include <memory>
#include <tuple>

/**
 * Tests with BreakpointModel and IDocumentController.
 *
 * Check BreakpointModel behaviour of Editing, Saving,
 * Discarding, Reloading, Renaming documents
 * with source line breakpoints set in them.
 */
class TestBreakpointModel : public QObject
{
    Q_OBJECT
public:
    explicit TestBreakpointModel(QObject* parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testDocumentSave();
    void testDocumentEditAndSave();
    void testDocumentEditAndDiscard();

private:
    using DocumentMarks = QHash<int, KDevelop::BreakpointModel::MarkType>;
    static DocumentMarks documentMarks(const KDevelop::IDocument* doc);
    static void printLines(int from, int count, const KDevelop::IDocument* doc);
    static void verifyBreakpoint(KDevelop::Breakpoint* breakpoint, int expectedLine, const DocumentMarks& marks);
    static void verifyUntrackedBreakpoint(KDevelop::Breakpoint* breakpoint, int expectedLine);

    QUrl testFileUrl(const QString& fileName) const;

    using DocumentAndBreakpoint = std::tuple<QUrl, KDevelop::IDocument*, KDevelop::Breakpoint*>;
    DocumentAndBreakpoint setupPrimaryDocumentAndBreakpoint();

    using DocumentAndTwoBreakpoints =
        std::tuple<QUrl, KDevelop::IDocument*, KDevelop::Breakpoint*, KDevelop::Breakpoint*>;
    DocumentAndTwoBreakpoints setupEditAndCheckPrimaryDocumentAndBreakpoints();

    std::unique_ptr<QTemporaryDir> m_tempDir; ///< Test working directory
};

#endif // KDEVPLATFORM_TEST_BREAKPOINTMODEL_H
