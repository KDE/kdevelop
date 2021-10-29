/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_vcslocation.h"

#include <QTest>
#include <QStandardPaths>

using namespace KDevelop;

void TestVcsLocation::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestVcsLocation::setServerLocation(VcsLocation& serverLocation,
                                        const QString& repositoryModule,
                                        const QString& repositoryBranch,
                                        const QString& repositoryTag,
                                        const QString& repositoryPath,
                                        const QVariant& userData)
{
    serverLocation.setRepositoryModule(repositoryModule);
    serverLocation.setRepositoryBranch(repositoryBranch);
    serverLocation.setRepositoryTag(repositoryTag);
    serverLocation.setRepositoryPath(repositoryPath);
    serverLocation.setUserData(userData);
}

void TestVcsLocation::compareServerLocation(const VcsLocation& serverLocation,
                                            const QString& repositoryServer,
                                            const QString& repositoryModule,
                                            const QString& repositoryBranch,
                                            const QString& repositoryTag,
                                            const QString& repositoryPath,
                                            const QVariant& userData)
{
    QCOMPARE(serverLocation.isValid(), true);
    QCOMPARE(serverLocation.type(), VcsLocation::RepositoryLocation);
    QCOMPARE(serverLocation.repositoryServer(), repositoryServer);
    QCOMPARE(serverLocation.repositoryModule(), repositoryModule);
    QCOMPARE(serverLocation.repositoryBranch(), repositoryBranch);
    QCOMPARE(serverLocation.repositoryTag(), repositoryTag);
    QCOMPARE(serverLocation.repositoryPath(), repositoryPath);
    QCOMPARE(serverLocation.userData(), userData);
}

void TestVcsLocation::testDefaultConstructor()
{
    const VcsLocation location;

    QCOMPARE(location.isValid(), false);
}

void TestVcsLocation::testLocalUrlConstructor()
{
    // valid
    {
        const QUrl localUrl = QUrl("file:///tmp/foo");

        const VcsLocation localLocation(localUrl);

        QCOMPARE(localLocation.isValid(), true);
        QCOMPARE(localLocation.type(), VcsLocation::LocalLocation);
        QCOMPARE(localLocation.localUrl(), localUrl);
    }

    // invalid
    {
        const QUrl localUrl;

        const VcsLocation localLocation(localUrl);

        QCOMPARE(localLocation.isValid(), false);
        QCOMPARE(localLocation.type(), VcsLocation::LocalLocation);
        QCOMPARE(localLocation.localUrl(), localUrl);
    }
}

void TestVcsLocation::testRepositoryServerConstructor()
{
    // valid
    {
        const QString repositoryServer = QStringLiteral("server");
        const QString repositoryModule = QStringLiteral("module");
        const QString repositoryBranch = QStringLiteral("branch");
        const QString repositoryTag = QStringLiteral("tag");
        const QString repositoryPath = QStringLiteral("path");
        const QVariant userData = QVariant(QStringLiteral("userdata"));

        VcsLocation serverLocation(repositoryServer);
        setServerLocation(serverLocation,
                          repositoryModule, repositoryBranch, repositoryTag, repositoryPath, userData);

        compareServerLocation(serverLocation,
                              repositoryServer, repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
    }

    // invalid
    {
        const QString repositoryServer;
        VcsLocation serverLocation(repositoryServer);

        QCOMPARE(serverLocation.isValid(), false);
        QCOMPARE(serverLocation.type(), VcsLocation::RepositoryLocation);
        QCOMPARE(serverLocation.repositoryServer(), repositoryServer);
    }
}

void TestVcsLocation::testCopyConstructor()
{
    // test plain copy
    const QString repositoryServer = QStringLiteral("server");
    const QString repositoryModule = QStringLiteral("module");
    const QString repositoryBranch = QStringLiteral("branch");
    const QString repositoryTag = QStringLiteral("tag");
    const QString repositoryPath = QStringLiteral("path");
    const QVariant userData = QVariant(QStringLiteral("userdata"));

    {
        VcsLocation serverLocationA(repositoryServer);
        setServerLocation(serverLocationA,
                          repositoryModule, repositoryBranch, repositoryTag, repositoryPath, userData);

        VcsLocation serverLocationB(serverLocationA);
        compareServerLocation(serverLocationA,
                              repositoryServer, repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
        compareServerLocation(serverLocationB,
                              repositoryServer, repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
        QVERIFY(serverLocationB == serverLocationA);
        QVERIFY(serverLocationA == serverLocationB);
    }

    const QString repositoryServerNew = QStringLiteral("servernew");

    // test detach after changing A
    {
        VcsLocation serverLocationA(repositoryServer);
        setServerLocation(serverLocationA,
                          repositoryModule, repositoryBranch, repositoryTag, repositoryPath, userData);

        VcsLocation serverLocationB(serverLocationA);
        // change a property of A
        serverLocationA.setRepositoryServer(repositoryServerNew);

        compareServerLocation(serverLocationA,
                              repositoryServerNew, repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
        compareServerLocation(serverLocationB,
                              repositoryServer,    repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
        QVERIFY(!(serverLocationB == serverLocationA));
        QVERIFY(!(serverLocationA == serverLocationB));
    }
}

void TestVcsLocation::testAssignOperator()
{
    // test plain copy
    const QString repositoryServer = QStringLiteral("server");
    const QString repositoryModule = QStringLiteral("module");
    const QString repositoryBranch = QStringLiteral("branch");
    const QString repositoryTag = QStringLiteral("tag");
    const QString repositoryPath = QStringLiteral("path");
    const QVariant userData = QVariant(QStringLiteral("userdata"));

    {
        VcsLocation serverLocationA(repositoryServer);
        setServerLocation(serverLocationA,
                          repositoryModule, repositoryBranch, repositoryTag, repositoryPath, userData);

        VcsLocation serverLocationB;
        serverLocationB = serverLocationA;
        compareServerLocation(serverLocationA,
                              repositoryServer, repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
        compareServerLocation(serverLocationB,
                              repositoryServer, repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
        QVERIFY(serverLocationB == serverLocationA);
        QVERIFY(serverLocationA == serverLocationB);
    }

    const QString repositoryServerNew = QStringLiteral("servernew");

    // test detach after changing A
    {
        VcsLocation serverLocationA(repositoryServer);
        setServerLocation(serverLocationA,
                          repositoryModule, repositoryBranch, repositoryTag, repositoryPath, userData);

        VcsLocation serverLocationB;
        serverLocationB = serverLocationA;
        // change a property of A
        serverLocationA.setRepositoryServer(repositoryServerNew);

        compareServerLocation(serverLocationA,
                              repositoryServerNew, repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
        compareServerLocation(serverLocationB,
                              repositoryServer,    repositoryModule, repositoryBranch, repositoryTag, repositoryPath,
                              userData);
        QVERIFY(!(serverLocationB == serverLocationA));
        QVERIFY(!(serverLocationA == serverLocationB));
    }
}

QTEST_GUILESS_MAIN(TestVcsLocation)
