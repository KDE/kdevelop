/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cmakeserver.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>
#include <tests/testcore.h>
#include "testhelpers.h"
#include <cmakeutils.h>

#include <QTest>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QVersionNumber>

using namespace KDevelop;

class CMakeServerTest : public QObject
{
    Q_OBJECT
public:
    CMakeServerTest()
    {
        QLoggingCategory::setFilterRules(QStringLiteral("default.debug=true\nkdevelop.projectmanagers.cmake.debug=true\n"));

        AutoTestShell::init({ "KDevCMakeManager", "KDevCMakeBuilder", "KDevMakeBuilder", "KDevStandardOutputView" });
        TestCore::initialize();
    }

private Q_SLOTS:
    void initTestCase()
    {
        const auto exe = CMake::findExecutable();
        QVERIFY(!exe.isEmpty());
        const auto versionStr = CMake::cmakeExecutableVersion(exe);
        QVERIFY(!versionStr.isEmpty());
        const auto version = QVersionNumber::fromString(versionStr);
        QVERIFY(!version.isNull());
        const bool versionWithServer = (version >= QVersionNumber(3, 8) && version < QVersionNumber(3, 20));
        if (!versionWithServer)
            QSKIP("cmake server not supported");
    }

    void testRun()
    {
        CMakeServer server(nullptr);
        QSignalSpy spyConnected(&server, &CMakeServer::connected);
        QVERIFY(server.isServerAvailable() || spyConnected.wait());

        QSignalSpy spy(&server, &CMakeServer::response);
        QJsonObject codeModel;
        int errors = 0;
        connect(&server, &CMakeServer::response, this, [&errors, &codeModel, &server](const QJsonObject &response) {
            if (response.value(QStringLiteral("type")) == QLatin1String("reply")) {
                if (response.value(QStringLiteral("inReplyTo")) == QLatin1String("configure"))
                    server.compute();
                else if (response.value(QStringLiteral("inReplyTo")) == QLatin1String("compute"))
                    server.codemodel();
                else if(response.value(QStringLiteral("inReplyTo")) == QLatin1String("codemodel"))
                    codeModel = response;
            } else if(response.value(QStringLiteral("type")) == QLatin1String("error")) {
                ++errors;
            }
        });

        const QString name = QStringLiteral("single_subdirectory");
        const auto paths = projectPaths(name);
        const QString builddir = QStringLiteral(CMAKE_TESTS_BINARY_DIR "/cmake-server-test-builddir/") + name;
        QVERIFY(QDir(builddir).removeRecursively());
        QVERIFY(QDir(builddir).mkpath(builddir));

        QVERIFY(spy.wait());
        server.handshake(paths.sourceDir, Path(builddir));
        QVERIFY(spy.wait());
        server.configure({});
        while(codeModel.isEmpty())
            QVERIFY(spy.wait());
        QCOMPARE(errors, 0);
        QVERIFY(!codeModel.isEmpty());
        qDebug() << "codemodel" << codeModel;
    }
};

QTEST_MAIN( CMakeServerTest )

#include "test_cmakeserver.moc"
