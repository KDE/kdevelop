/* This file is part of KDevelop
 *
 * Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "test_vcsannotation.h"

#include <QTest>

#include <vcs/vcsannotation.h>
#include <vcs/vcsrevision.h>

using namespace KDevelop;

static
VcsAnnotationLine createAnnotationLine(int lineNumber,
                                       const QString& text,
                                       const QString& author,
                                       const VcsRevision& revision,
                                       const QDateTime& date,
                                       const QString& commitMessage)
{
    VcsAnnotationLine annotationLine;
    annotationLine.setLineNumber(lineNumber);
    annotationLine.setText(text);
    annotationLine.setAuthor(author);
    annotationLine.setRevision(revision);
    annotationLine.setDate(date);
    annotationLine.setCommitMessage(commitMessage);
    return annotationLine;
}

void TestVcsAnnotation::testCopyConstructor()
{
    const int lineNumber = 1;
    const QString text("Text A");
    const QString author("Author A");
    VcsRevision revision;
    revision.setRevisionValue("A", VcsRevision::GlobalNumber);
    const QDateTime date = QDateTime::fromString("2001-01-01T00:00:00+00:00", Qt::ISODate);
    const QString commitMessage("Commit A");
    const VcsAnnotationLine annotationLine = createAnnotationLine(lineNumber, text, author, revision, date, commitMessage);
    const QUrl location(QStringLiteral("git://foo"));

    // test plain copy
    {
        VcsAnnotation annotationA;
        annotationA.setLocation(location);
        annotationA.insertLine(lineNumber, annotationLine);

        VcsAnnotation annotationB(annotationA);

        QCOMPARE(annotationA.location(), location);
        QCOMPARE(annotationA.lineCount(), 1);
        QVERIFY(annotationA.containsLine(lineNumber));
        QCOMPARE(annotationB.location(), location);
        QCOMPARE(annotationB.lineCount(), 1);
        QVERIFY(annotationB.containsLine(lineNumber));
    }

    const QUrl locationNew(QStringLiteral("svn://bar"));

    // test detach after changing A
    {
        VcsAnnotation annotationA;
        annotationA.setLocation(location);
        annotationA.insertLine(lineNumber, annotationLine);

        VcsAnnotation annotationB(annotationA);
        // change a property of A
        annotationA.setLocation(locationNew);

        QCOMPARE(annotationA.location(), locationNew);
        QCOMPARE(annotationA.lineCount(), 1);
        QVERIFY(annotationA.containsLine(lineNumber));
        QCOMPARE(annotationB.location(), location);
        QCOMPARE(annotationB.lineCount(), 1);
        QVERIFY(annotationB.containsLine(lineNumber));
    }
}

void TestVcsAnnotation::testAssignOperator()
{
    const int lineNumber = 1;
    const QString text("Text A");
    const QString author("Author A");
    VcsRevision revision;
    revision.setRevisionValue("A", VcsRevision::GlobalNumber);
    const QDateTime date = QDateTime::fromString("2001-01-01T00:00:00+00:00", Qt::ISODate);
    const QString commitMessage("Commit A");
    const VcsAnnotationLine annotationLine = createAnnotationLine(lineNumber, text, author, revision, date, commitMessage);
    const QUrl location(QStringLiteral("https://kdevelop.org"));

    // test plain copy
    {
        VcsAnnotation annotationA;
        annotationA.setLocation(location);
        annotationA.insertLine(lineNumber, annotationLine);

        VcsAnnotation annotationB;
        annotationB = annotationA;

        QCOMPARE(annotationA.location(), location);
        QCOMPARE(annotationA.lineCount(), 1);
        QVERIFY(annotationA.containsLine(lineNumber));
        QCOMPARE(annotationB.location(), location);
        QCOMPARE(annotationB.lineCount(), 1);
        QVERIFY(annotationB.containsLine(lineNumber));
    }

    const QUrl locationNew(QStringLiteral("https://kate-editor.org"));

    // test detach after changing A
    {
        VcsAnnotation annotationA;
        annotationA.setLocation(location);
        annotationA.insertLine(lineNumber, annotationLine);

        VcsAnnotation annotationB;
        annotationB = annotationA;
        // change a property of A
        annotationA.setLocation(locationNew);

        QCOMPARE(annotationA.location(), locationNew);
        QCOMPARE(annotationA.lineCount(), 1);
        QVERIFY(annotationA.containsLine(lineNumber));
        QCOMPARE(annotationB.location(), location);
        QCOMPARE(annotationB.lineCount(), 1);
        QVERIFY(annotationB.containsLine(lineNumber));
    }
}

QTEST_GUILESS_MAIN(TestVcsAnnotation)
