/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTVCSDIFF_H
#define KDEVPLATFORM_TESTVCSDIFF_H

#include <QObject>

#include "vcsdiff.h"

class TestVcsDiff : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCopyConstructor();
    void testAssignOperator();
    void testFileNames();
    void testSubDiff();
    void testLineMapping();
    void testConflicts();

private:
    QString sampleDiff;

    void setDiff(KDevelop::VcsDiff& diff,
                 const QString& diffString,
                 const QUrl& baseDiff,
                 uint depth);
    void compareDiff(const KDevelop::VcsDiff& diff,
                     const QString& diffString,
                     const QUrl& baseDiff,
                     uint depth);
};

#endif // KDEVPLATFORM_TESTVCSDIFF_H
