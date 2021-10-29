/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTVCSLOCATION_H
#define KDEVPLATFORM_TESTVCSLOCATION_H

#include <QObject>

#include "vcslocation.h"

class TestVcsLocation : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testDefaultConstructor();
    void testLocalUrlConstructor();
    void testRepositoryServerConstructor();

    void testCopyConstructor();
    void testAssignOperator();

private:
    void setServerLocation(KDevelop::VcsLocation& serverLocation,
                           const QString& repositoryModule,
                           const QString& repositoryBranch,
                           const QString& repositoryTag,
                           const QString& repositoryPath,
                           const QVariant& userData);
    void compareServerLocation(const KDevelop::VcsLocation& serverLocation,
                               const QString& repositoryServer,
                               const QString& repositoryModule,
                               const QString& repositoryBranch,
                               const QString& repositoryTag,
                               const QString& repositoryPath,
                               const QVariant& userData);
};

#endif // KDEVPLATFORM_TESTVCSLOCATION_H
