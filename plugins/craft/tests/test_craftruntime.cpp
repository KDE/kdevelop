// SPDX-FileCopyrightText: 2022 Gleb Popov <arrowd@FreeBSD.org>
// SPDX-License-Identifier: BSD-3-Clause

#include "test_craftruntime.h"

#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>

#include <KIO/CopyJob>
#include <KProcess>

#include <tests/testcore.h>
#include <tests/testhelpers.h>

#include "../craftruntime.h"

using namespace KDevelop;

QTEST_MAIN(CraftRuntimeTest)

class TempDirWrapper
{
public:
    TempDirWrapper() = default;
    TempDirWrapper(const QString& craftRoot, const QString& pythonExecutable)
        : m_tempCraftRoot(new QTemporaryDir())
    {
        QVERIFY(m_tempCraftRoot->isValid());
        copyCraftRoot(craftRoot);
        m_runtime = std::make_shared<CraftRuntime>(m_tempCraftRoot->path(), pythonExecutable);
    }

    QString path() const
    {
        QVERIFY_RETURN(m_tempCraftRoot, QString());
        return m_tempCraftRoot->path();
    }

    CraftRuntime* operator->() const
    {
        QVERIFY_RETURN(m_runtime, nullptr);
        return m_runtime.get();
    }

private:
    void copyCraftRoot(const QString& oldRoot) const
    {
        const QLatin1String craftSettingsRelativePath("/etc/CraftSettings.ini");
        const QDir dest(m_tempCraftRoot->path());

        auto* job = KIO::copy(QUrl::fromLocalFile(oldRoot + QLatin1String("/craft")), QUrl::fromLocalFile(dest.path()));
        QVERIFY(job->exec());

        QVERIFY(dest.mkpath(QLatin1String("bin")));
        QVERIFY(dest.mkpath(QLatin1String("etc")));

        QVERIFY(QFile::copy(oldRoot + craftSettingsRelativePath, dest.path() + craftSettingsRelativePath));
    }
    std::shared_ptr<CraftRuntime> m_runtime;
    std::shared_ptr<QTemporaryDir> m_tempCraftRoot;
};

Q_DECLARE_METATYPE(TempDirWrapper)

// When this test itself is ran under a Craft root, its environment gets in the way
static void breakoutFromCraftRoot()
{
    auto craftRoot = qgetenv("KDEROOT");
    if (craftRoot.isEmpty())
        return;

    auto paths = qgetenv("PATH").split(':');
    const auto it = std::remove_if(paths.begin(), paths.end(), [craftRoot](const QByteArray& path) {
        return path.startsWith(craftRoot);
    });
    paths.erase(it, paths.end());
    qputenv("PATH", paths.join(':'));

    qunsetenv("KDEROOT");
    qunsetenv("craftRoot");
}

void CraftRuntimeTest::initTestCase_data()
{
    breakoutFromCraftRoot();

    const QString pythonExecutable = CraftRuntime::findPython();
    if (pythonExecutable.isEmpty())
        QSKIP("No python found, skipping kdevcraft tests.");

    QTest::addColumn<TempDirWrapper>("runtimeInstance");

    QTest::newRow("Mock") << TempDirWrapper(QStringLiteral(CRAFT_ROOT_MOCK), pythonExecutable);

    auto craftRoot = CraftRuntime::findCraftRoot(Path(QStringLiteral(".")));
    if (!craftRoot.isEmpty())
        QTest::newRow("Real") << TempDirWrapper(craftRoot, pythonExecutable);
}

void CraftRuntimeTest::testFindCraftRoot()
{
    QFETCH_GLOBAL(TempDirWrapper, runtimeInstance);
    QCOMPARE(CraftRuntime::findCraftRoot(Path(runtimeInstance.path())), runtimeInstance.path());
    QCOMPARE(CraftRuntime::findCraftRoot(Path(runtimeInstance.path()).cd(QStringLiteral("bin"))),
             runtimeInstance.path());
}

void CraftRuntimeTest::testGetenv()
{
    QFETCH_GLOBAL(TempDirWrapper, runtimeInstance);

    QVERIFY(!runtimeInstance->getenv("KDEROOT").isEmpty());

    QDir craftDir1 = QDir(QString::fromLocal8Bit(runtimeInstance->getenv("KDEROOT")));
    QDir craftDir2 = QDir(runtimeInstance.path());
    QCOMPARE(craftDir1.canonicalPath(), craftDir2.canonicalPath());

    QString pythonpathValue = QString::fromLocal8Bit(runtimeInstance->getenv("PYTHONPATH"));
    QVERIFY(!pythonpathValue.isEmpty());
    QDir craftPythonPathDir = QDir(pythonpathValue);

    QVERIFY(craftPythonPathDir.path().startsWith(craftDir1.path()));
}

void CraftRuntimeTest::testStartProcess()
{
    QFETCH_GLOBAL(TempDirWrapper, runtimeInstance);

    QString envPath = QStandardPaths::findExecutable(QStringLiteral("env"));
    if (envPath.isEmpty())
        QSKIP("Skipping startProcess() test, no \"env\" executable found");

    QString envUnderCraftPath = runtimeInstance.path() + QStringLiteral("/bin/env");
    QVERIFY(QFile::copy(envPath, envUnderCraftPath));

    QProcess p;
    p.setProgram(QStringLiteral("env"));
    runtimeInstance->startProcess(&p);

    // test that CraftRuntime::startProcess prefers programs under Craft root
    QCOMPARE(QDir(p.program()).canonicalPath(), QDir(envUnderCraftPath).canonicalPath());

    p.waitForFinished();
    QVERIFY(QFile::remove(envUnderCraftPath));
}

void CraftRuntimeTest::testStartProcessEnv()
{
    QFETCH_GLOBAL(TempDirWrapper, runtimeInstance);

    QString printenvPath = QStandardPaths::findExecutable(QStringLiteral("printenv"));
    if (printenvPath.isEmpty())
        QSKIP("Skipping startProcess() test, no \"printenv\" executable found");

    QString printenvUnderCraftPath = runtimeInstance.path() + QStringLiteral("/bin/printenv");
    QVERIFY(QFile::copy(printenvPath, printenvUnderCraftPath));

    KProcess p;
    p.setProgram(QStringLiteral("printenv"), QStringList{QStringLiteral("PYTHONPATH")});
    p.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    runtimeInstance->startProcess(&p);
    p.waitForFinished();

    QVERIFY(p.readAllStandardOutput().contains("site-packages"));
}

#include "moc_test_craftruntime.cpp"
