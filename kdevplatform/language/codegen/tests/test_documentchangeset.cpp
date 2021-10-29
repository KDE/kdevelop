/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_documentchangeset.h"

#include <language/codegen/documentchangeset.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <QTest>

QTEST_GUILESS_MAIN(TestDocumentchangeset)

using namespace KDevelop;

void TestDocumentchangeset::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestDocumentchangeset::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDocumentchangeset::testReplaceSameLine()
{
    TestFile file(QStringLiteral("abc def ghi"), QStringLiteral("cpp"));
    qDebug() << file.fileContents();
    DocumentChangeSet changes;
    changes.addChange(
        DocumentChange(
            file.url(),
            KTextEditor::Range(0, 0, 0, 3),
            QStringLiteral("abc"), QStringLiteral("foobar")
    ));
    changes.addChange(
        DocumentChange(
            file.url(),
            KTextEditor::Range(0, 4, 0, 7),
            QStringLiteral("def"), QStringLiteral("foobar")
    ));
    changes.addChange(
        DocumentChange(
            file.url(),
            KTextEditor::Range(0, 8, 0, 11),
            QStringLiteral("ghi"), QStringLiteral("foobar")
    ));

    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    qDebug() << result.m_failureReason << result.m_success;
    QVERIFY(result);
}

