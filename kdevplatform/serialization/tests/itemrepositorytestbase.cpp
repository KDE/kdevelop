/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "itemrepositorytestbase.h"

#include <serialization/itemrepositoryregistry.h>

#include <QStandardPaths>
#include <QTest>

using namespace KDevelop;

void ItemRepositoryTestBase::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    QVERIFY2(m_repositoryDir.isValid(),
             qPrintable("couldn't create temporary directory: " + m_repositoryDir.errorString()));
    ItemRepositoryRegistry::initialize(m_repositoryDir.path());
}

void ItemRepositoryTestBase::cleanupTestCase()
{
    if (!m_repositoryDir.isValid()) {
        return; // the repository has not been initialized => skip the cleanup
    }
    ItemRepositoryRegistry::deleteRepositoryFromDisk(m_repositoryDir.path());
    globalItemRepositoryRegistry().shutdown();
}
