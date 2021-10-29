/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_vcsannotation.h"

#include <QTest>
#include <QStandardPaths>

#include <vcs/vcsannotation.h>
#include <vcs/vcsrevision.h>

using namespace KDevelop;

void TestVcsAnnotation::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

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
