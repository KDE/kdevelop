/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_vcsannotationline.h"

#include <QTest>
#include <QStandardPaths>

#include <vcs/vcsannotation.h>
#include <vcs/vcsrevision.h>

using namespace KDevelop;

void TestVcsAnnotationLine::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestVcsAnnotationLine::setAnnotationLine(VcsAnnotationLine& annotationLine,
                                              int lineNumber,
                                              const QString& text,
                                              const QString& author,
                                              const VcsRevision& revision,
                                              const QDateTime& date,
                                              const QString& commitMessage)
{
    annotationLine.setLineNumber(lineNumber);
    annotationLine.setText(text);
    annotationLine.setAuthor(author);
    annotationLine.setRevision(revision);
    annotationLine.setDate(date);
    annotationLine.setCommitMessage(commitMessage);
}

void TestVcsAnnotationLine::compareAnnotationLine(const VcsAnnotationLine& annotationLine,
                                                  int lineNumber,
                                                  const QString& text,
                                                  const QString& author,
                                                  const VcsRevision& revision,
                                                  const QDateTime& date,
                                                  const QString& commitMessage)
{
    QCOMPARE(annotationLine.lineNumber(), lineNumber);
    QCOMPARE(annotationLine.text(), text);
    QCOMPARE(annotationLine.author(), author);
    QCOMPARE(annotationLine.revision(), revision);
    QCOMPARE(annotationLine.date(), date);
    QCOMPARE(annotationLine.commitMessage(), commitMessage);
}

void TestVcsAnnotationLine::testCopyConstructor()
{
    // copy invalid
    {
        VcsAnnotationLine annotationLineA;

        VcsAnnotationLine annotationLineB(annotationLineA);
        QCOMPARE(annotationLineA.revision().revisionType(), VcsRevision::Invalid);
        QCOMPARE(annotationLineB.revision().revisionType(), VcsRevision::Invalid);
    }

    // test plain copy
    const int lineNumber = 1;
    const QString text("Text A");
    const QString author("Author A");
    VcsRevision revision;
    revision.setRevisionValue("A", VcsRevision::GlobalNumber);
    const QDateTime date = QDateTime::fromString("2001-01-01T00:00:00+00:00", Qt::ISODate);
    const QString commitMessage("Commit A");

    {
        VcsAnnotationLine annotationLineA;
        setAnnotationLine(annotationLineA, 
                          lineNumber, text, author, revision, date, commitMessage);

        VcsAnnotationLine annotationLineB(annotationLineA);
        compareAnnotationLine(annotationLineA,
                              lineNumber, text, author, revision, date, commitMessage);
        compareAnnotationLine(annotationLineB,
                              lineNumber, text, author, revision, date, commitMessage);
    }

    const int lineNumberNew = 10;

    // test detach after changing A
    {
        VcsAnnotationLine annotationLineA;
        setAnnotationLine(annotationLineA, 
                          lineNumber, text, author, revision, date, commitMessage);

        VcsAnnotationLine annotationLineB(annotationLineA);
        // change a property of A
        annotationLineA.setLineNumber(lineNumberNew);

        compareAnnotationLine(annotationLineA,
                              lineNumberNew, text, author, revision, date, commitMessage);
        compareAnnotationLine(annotationLineB,
                              lineNumber,    text, author, revision, date, commitMessage);
    }
}

void TestVcsAnnotationLine::testAssignOperator()
{
    // assign invalid
    {
        VcsAnnotationLine annotationLineA;

        VcsAnnotationLine annotationLineB;
        VcsRevision revision;
        revision.setRevisionValue(2, VcsRevision::FileNumber);
        annotationLineB.setRevision(revision);

        annotationLineB = annotationLineA;

        QCOMPARE(annotationLineA.revision().revisionType(), VcsRevision::Invalid);
        QCOMPARE(annotationLineB.revision().revisionType(), VcsRevision::Invalid);
    }

    // test plain assign
    const int lineNumber = 1;
    const QString text("Text A");
    const QString author("Author A");
    VcsRevision revision;
    revision.setRevisionValue("A", VcsRevision::GlobalNumber);
    const QDateTime date = QDateTime::fromString("2001-01-01T00:00:00+00:00", Qt::ISODate);
    const QString commitMessage("Commit A");

    {
        VcsAnnotationLine annotationLineA;
        setAnnotationLine(annotationLineA, 
                          lineNumber, text, author, revision, date, commitMessage);

        VcsAnnotationLine annotationLineB;
        annotationLineB = annotationLineA;

        compareAnnotationLine(annotationLineA,
                              lineNumber, text, author, revision, date, commitMessage);
        compareAnnotationLine(annotationLineB,
                              lineNumber, text, author, revision, date, commitMessage);
    }

    const int lineNumberNew = 10;

    // test detach after changing A
    {
        VcsAnnotationLine annotationLineA;
        setAnnotationLine(annotationLineA, 
                          lineNumber, text, author, revision, date, commitMessage);

        VcsAnnotationLine annotationLineB;
        annotationLineB = annotationLineA;
        // change a property of A
        annotationLineA.setLineNumber(lineNumberNew);

        compareAnnotationLine(annotationLineA,
                              lineNumberNew, text, author, revision, date, commitMessage);
        compareAnnotationLine(annotationLineB,
                              lineNumber,    text, author, revision, date, commitMessage);
    }
}

QTEST_GUILESS_MAIN(TestVcsAnnotationLine)
