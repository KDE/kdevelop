/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test_documentchangeset.h"

#include <language/codegen/documentchangeset.h>

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

QTEST_KDEMAIN(TestDocumentchangeset, NoGUI);

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
    TestFile file("abc def ghi", "cpp");
    qDebug() << file.fileContents();
    DocumentChangeSet changes;
    changes.addChange(
        DocumentChange(
            file.url(),
            SimpleRange(0, 0, 0, 3),
            "abc", "foobar"
    ));
    changes.addChange(
        DocumentChange(
            file.url(),
            SimpleRange(0, 4, 0, 7),
            "def", "foobar"
    ));
    changes.addChange(
        DocumentChange(
            file.url(),
            SimpleRange(0, 8, 0, 11),
            "ghi", "foobar"
    ));

    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    qDebug() << result.m_failureReason << result.m_success;
    QVERIFY(result);
}

#include "test_documentchangeset.moc"
