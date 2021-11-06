/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_ITEMREPOSITORYTESTBASE_H
#define KDEVPLATFORM_ITEMREPOSITORYTESTBASE_H

#include <QObject>
#include <QTemporaryDir>

class ItemRepositoryTestBase : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private:
    const QTemporaryDir m_repositoryDir;
};

#endif // KDEVPLATFORM_ITEMREPOSITORYTESTBASE_H
