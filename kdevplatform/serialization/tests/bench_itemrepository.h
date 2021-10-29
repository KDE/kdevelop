/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTITEMREPOSITORY_H
#define TESTITEMREPOSITORY_H

#include <QDir>
#include <QObject>

class BenchItemRepository
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void insert();
    void remove();
    void removeDisk();
    void lookupKey();
    void lookupValue();

    void shouldDoReferenceCounting_data();
    void shouldDoReferenceCounting();

private:
    const QString m_repositoryPath = QDir::tempPath() + QStringLiteral("/bench_itemrepository");
};

#endif // TESTITEMREPOSITORY_H
