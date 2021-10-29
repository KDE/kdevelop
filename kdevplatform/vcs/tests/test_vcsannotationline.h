/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTVCSANNOTATIONLINE_H
#define KDEVPLATFORM_TESTVCSANNOTATIONLINE_H

#include <QObject>

namespace KDevelop {
class VcsAnnotationLine;
class VcsRevision;
}
class QDateTime;
class QString;

class TestVcsAnnotationLine : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCopyConstructor();
    void testAssignOperator();

private:
    void setAnnotationLine(KDevelop::VcsAnnotationLine& annotationLine,
                           int lineNumber,
                           const QString& text,
                           const QString& author,
                           const KDevelop::VcsRevision& revision,
                           const QDateTime& date,
                           const QString& commitMessage);
    void compareAnnotationLine(const KDevelop::VcsAnnotationLine& annotationLine,
                               int lineNumber,
                               const QString& text,
                               const QString& author,
                               const KDevelop::VcsRevision& revision,
                               const QDateTime& date,
                               const QString& commitMessage);
};

#endif // KDEVPLATFORM_TESTVCSANNOTATIONLINE_H
