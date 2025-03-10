/*
    This file was partly taken from KDevelop's cvs plugin
    SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>

    Adapted for Git
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_git.h"

#include "../gitplugin.h"

#include <tests/autotestshell.h>
#include <tests/plugintesthelpers.h>
#include <tests/testcore.h>
#include <tests/testhelpermacros.h>
#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include <vcs/vcsevent.h>

#include <KPluginMetaData>

#include <QDebug>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>

#include <algorithm>
#include <array>
#include <iterator>
#include <vector>

#define VERIFYJOB(j) \
do { QVERIFY(j); QVERIFY(j->exec()); QVERIFY((j)->status() == KDevelop::VcsJob::JobSucceeded); } while(0)

#define VERIFYJOBFAILURE(j) \
do { QVERIFY(j); QVERIFY(((!j->exec()) || ((j)->status() != KDevelop::VcsJob::JobSucceeded))); } while(0)

inline QString tempDir() { return QDir::tempPath(); }
inline QString gitTest_BaseDir() { return tempDir() + "/kdevGit_testdir/"; }
inline QString gitRepo() { return gitTest_BaseDir() + ".git"; }
inline QString gitSrcDir() { return gitTest_BaseDir() + "src/"; }
inline QString gitTest_FileName() { return QStringLiteral("testfile"); }
inline QString gitTest_FileName2() { return QStringLiteral("foo"); }
inline QString gitTest_FileName3() { return QStringLiteral("bar"); }

using namespace KDevelop;

bool writeFile(const QString &path, const QString& content, QIODevice::OpenModeFlag mode = QIODevice::WriteOnly)
{
    QFile f(path);

    if (!f.open(mode)) {
        return false;
    }

    QTextStream input(&f);
    input << content;

    return true;
}

void GitInitTest::initTestCase()
{
    AutoTestShell::init({QStringLiteral("kdevgit")});
    TestCore::initialize();

    const auto pluginMetaData = makeTestPluginMetaData("TestGit");
    m_plugin = new GitPlugin(TestCore::self(), pluginMetaData);

    removeTempDirs(); // in case the previous test run crashed and left a git repository behind
}

void GitInitTest::cleanupTestCase()
{
    delete m_plugin;

    TestCore::shutdown();
}

void GitInitTest::init()
{
    // Now create the basic directory structure
    QDir tmpdir(tempDir());
    tmpdir.mkdir(gitTest_BaseDir());
    tmpdir.mkdir(gitSrcDir());
}

void GitInitTest::cleanup()
{
    removeTempDirs();
}


void GitInitTest::repoInit()
{
    qDebug() << "Trying to init repo";
    // make job that creates the local repository
    VcsJob* j = m_plugin->init(QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    //check if the CVSROOT directory in the new local repository exists now
    QVERIFY(QFileInfo::exists(gitRepo()));

    //check if isValidDirectory works
    QVERIFY(m_plugin->isValidDirectory(QUrl::fromLocalFile(gitTest_BaseDir())));
    //and for non-git dir, I hope nobody has /tmp under git
    QVERIFY(!m_plugin->isValidDirectory(QUrl::fromLocalFile(tempDir())));

    //we have nothing, so output should be empty
    DVcsJob * j2 = m_plugin->gitRevParse(gitRepo(), QStringList(QStringLiteral("--branches")));
    QVERIFY(j2);
    QVERIFY(j2->exec());
    QVERIFY(j2->output().isEmpty());

    // Make sure to set the Git identity so unit tests don't depend on that
    auto j3 = m_plugin->setConfigOption(QUrl::fromLocalFile(gitTest_BaseDir()),
              QStringLiteral("user.email"), QStringLiteral("me@example.com"));
    VERIFYJOB(j3);
    auto j4 = m_plugin->setConfigOption(QUrl::fromLocalFile(gitTest_BaseDir()),
              QStringLiteral("user.name"), QStringLiteral("My Name"));
    VERIFYJOB(j4);
}

void GitInitTest::addFiles()
{
    qDebug() << "Adding files to the repo";

    //we start it after repoInit, so we still have empty git repo
    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName(), QStringLiteral("HELLO WORLD")));
    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName2(), QStringLiteral("No, bar()!")));

    //test git-status exitCode (see DVcsJob::setExitCode).
    VcsJob* j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    // /tmp/kdevGit_testdir/ and testfile
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir() + gitTest_FileName()));
    VERIFYJOB(j);

    QVERIFY(writeFile(gitSrcDir() + gitTest_FileName3(), QStringLiteral("No, foo()! It's bar()!")));

    //test git-status exitCode again
    j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    //repository path without trailing slash and a file in a parent directory
    // /tmp/repo  and /tmp/repo/src/bar
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitSrcDir() + gitTest_FileName3()));
    VERIFYJOB(j);

    //let's use absolute path, because it's used in ContextMenus
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir() + gitTest_FileName2()));
    VERIFYJOB(j);

    //Now let's create several files and try "git add file1 file2 file3"
    const QStringList files{QStringLiteral("file1"), QStringLiteral("file2"), QStringLiteral("la la")};
    QList<QUrl> multipleFiles;
    for (const QString& file : files) {
        QVERIFY(writeFile(gitTest_BaseDir() + file, file));
        multipleFiles << QUrl::fromLocalFile(gitTest_BaseDir() + file);
    }
    j = m_plugin->add(multipleFiles);
    VERIFYJOB(j);
}

void GitInitTest::commitFiles()
{
    qDebug() << "Committing...";
    //we start it after addFiles, so we just have to commit
    VcsJob* j = m_plugin->commit(QStringLiteral("Test commit"), QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    //test git-status exitCode one more time.
    j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    //since we committed the file to the "pure" repository, .git/refs/heads/master should exist
    //TODO: maybe other method should be used
    QString headRefName(gitRepo() + "/refs/heads/master");
    QVERIFY(QFileInfo::exists(headRefName));

    //Test the results of the "git add"
    auto* jobLs = new DVcsJob(gitTest_BaseDir(), m_plugin);
    *jobLs << "git" << "ls-tree" << "--name-only" << "-r" << "HEAD";

    if (jobLs->exec() && jobLs->status() == KDevelop::VcsJob::JobSucceeded) {
        QStringList files = jobLs->output().split('\n');
        QVERIFY(files.contains(gitTest_FileName()));
        QVERIFY(files.contains(gitTest_FileName2()));
        QVERIFY(files.contains("src/" + gitTest_FileName3()));
    }

    QString firstCommit;

    QFile headRef(headRefName);

    if (headRef.open(QIODevice::ReadOnly)) {
        QTextStream output(&headRef);
        output >> firstCommit;
    }
    headRef.close();

    QVERIFY(!firstCommit.isEmpty());

    qDebug() << "Committing one more time";
    //let's try to change the file and test "git commit -a"
    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName(), QStringLiteral("Just another HELLO WORLD\n")));

    //add changes
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir() + gitTest_FileName()));
    VERIFYJOB(j);

    j = m_plugin->commit(QStringLiteral("KDevelop's Test commit2"), QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    QString secondCommit;

    if (headRef.open(QIODevice::ReadOnly)) {
        QTextStream output(&headRef);
        output >> secondCommit;
    }
    headRef.close();

    QVERIFY(!secondCommit.isEmpty());
    QVERIFY(firstCommit != secondCommit);

}

void GitInitTest::testInit()
{
    repoInit();
}

static QString runCommand(const QString& cmd, const QStringList& args)
{
    QProcess proc;
    proc.setWorkingDirectory(gitTest_BaseDir());
    proc.start(cmd, args);
    proc.waitForFinished();
    return proc.readAllStandardOutput().trimmed();
}

void GitInitTest::testReadAndSetConfigOption()
{
    repoInit();

    {
        qDebug() << "read non-existing config option";
        QString nameFromPlugin = m_plugin->readConfigOption(QUrl::fromLocalFile(gitTest_BaseDir()),
                                                            QStringLiteral("notexisting.asdads"));
        QVERIFY(nameFromPlugin.isEmpty());
    }

    {
        qDebug() << "write user.name = \"John Tester\"";
        auto job = m_plugin->setConfigOption(QUrl::fromLocalFile(gitTest_BaseDir()),
                                             QStringLiteral("user.name"), QStringLiteral("John Tester"));
        VERIFYJOB(job);
        const auto name = runCommand(QStringLiteral("git"), {"config", "--get", QStringLiteral("user.name")});
        QCOMPARE(name, QStringLiteral("John Tester"));
    }

    {
        qDebug() << "read user.name";
        const QString nameFromPlugin = m_plugin->readConfigOption(QUrl::fromLocalFile(gitTest_BaseDir()),
                                                               QStringLiteral("user.name"));
        QCOMPARE(nameFromPlugin, QStringLiteral("John Tester"));
        const auto name = runCommand(QStringLiteral("git"), {"config", "--get", QStringLiteral("user.name")});
        QCOMPARE(name, QStringLiteral("John Tester"));
    }
}

void GitInitTest::testAdd()
{
    repoInit();
    addFiles();
}

void GitInitTest::testCommit()
{
    repoInit();
    addFiles();
    commitFiles();
}

void GitInitTest::testBranch(const QString& newBranch)
{
    //Already tested, so I assume that it works
    const QUrl baseUrl = QUrl::fromLocalFile(gitTest_BaseDir());
    QString oldBranch = runSynchronously(m_plugin->currentBranch(baseUrl)).toString();

    VcsRevision rev;
    rev.setRevisionValue(oldBranch, KDevelop::VcsRevision::GlobalNumber);
    VcsJob* j = m_plugin->branch(baseUrl, rev, newBranch);
    VERIFYJOB(j);
    QVERIFY(runSynchronously(m_plugin->branches(baseUrl)).toStringList().contains(newBranch));

    // switch branch
    j = m_plugin->switchBranch(baseUrl, newBranch);
    VERIFYJOB(j);
    QCOMPARE(runSynchronously(m_plugin->currentBranch(baseUrl)).toString(), newBranch);

    // get into detached head state
    j = m_plugin->switchBranch(baseUrl, QStringLiteral("HEAD~1"));
    VERIFYJOB(j);
    QCOMPARE(runSynchronously(m_plugin->currentBranch(baseUrl)).toString(), QString());

    // switch back
    j = m_plugin->switchBranch(baseUrl, newBranch);
    VERIFYJOB(j);
    QCOMPARE(runSynchronously(m_plugin->currentBranch(baseUrl)).toString(), newBranch);

    j = m_plugin->deleteBranch(baseUrl, oldBranch);
    VERIFYJOB(j);
    QVERIFY(!runSynchronously(m_plugin->branches(baseUrl)).toStringList().contains(oldBranch));

    // Test that we can't delete branch on which we currently are
    j = m_plugin->deleteBranch(baseUrl, newBranch);
    VERIFYJOBFAILURE(j);
    QVERIFY(runSynchronously(m_plugin->branches(baseUrl)).toStringList().contains(newBranch));

    // Test branching off HEAD
    rev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Head);
    j = m_plugin->branch(baseUrl, rev, newBranch+QStringLiteral("-new"));
    VERIFYJOB(j);
    QVERIFY(runSynchronously(m_plugin->branches(baseUrl)).toStringList().contains(newBranch+QStringLiteral("-new")));
}

void GitInitTest::testMerge()
{
    const QString branchNames[] = {QStringLiteral("aBranchToBeMergedIntoMaster"), QStringLiteral("AnotherBranch")};

    const QString files[]={QStringLiteral("First File to Appear after merging"),QStringLiteral("Second File to Appear after merging"), QStringLiteral("Another_File.txt")};

    const QString content=QStringLiteral("Testing merge.");

    repoInit();
    addFiles();
    commitFiles();

    const QUrl baseUrl = QUrl::fromLocalFile(gitTest_BaseDir());
    VcsJob* j = m_plugin->branches(baseUrl);
    VERIFYJOB(j);
    QString curBranch = runSynchronously(m_plugin->currentBranch(baseUrl)).toString();
    QCOMPARE(curBranch, QStringLiteral("master"));

    VcsRevision rev;
    rev.setRevisionValue("master", KDevelop::VcsRevision::GlobalNumber);

    j = m_plugin->branch(baseUrl, rev, branchNames[0]);
    VERIFYJOB(j);
    j = m_plugin->switchBranch(baseUrl, branchNames[0]);
    VERIFYJOB(j);

    qDebug() << "Adding files to the new branch";

    //we start it after repoInit, so we still have empty git repo
    QVERIFY(writeFile(gitTest_BaseDir() + files[0], content));
    QVERIFY(writeFile(gitTest_BaseDir() + files[1], content));

    QList<QUrl> listOfAddedFiles{QUrl::fromLocalFile(gitTest_BaseDir() + files[0]),
        QUrl::fromLocalFile(gitTest_BaseDir() + files[1])};

    j = m_plugin->add(listOfAddedFiles);
    VERIFYJOB(j);

    j = m_plugin->commit(QStringLiteral("Committing to the new branch"), QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    j = m_plugin->switchBranch(baseUrl, QStringLiteral("master"));
    VERIFYJOB(j);

    j = m_plugin->mergeBranch(baseUrl, branchNames[0]);
    VERIFYJOB(j);

    auto jobLs = new DVcsJob(gitTest_BaseDir(), m_plugin);
    *jobLs << "git" << "ls-tree" << "--name-only" << "-r" << "HEAD" ;

    if (jobLs->exec() && jobLs->status() == KDevelop::VcsJob::JobSucceeded) {
        QStringList lsFiles = jobLs->output().split('\n');
        qDebug() << "Files in this Branch: " << lsFiles;
        QVERIFY(lsFiles.contains(files[0]));
        QVERIFY(lsFiles.contains(files[1]));
    }

    //Testing one more time.
    j = m_plugin->switchBranch(baseUrl, branchNames[0]);
    VERIFYJOB(j);
    rev.setRevisionValue(branchNames[0], KDevelop::VcsRevision::GlobalNumber);
    j = m_plugin->branch(baseUrl, rev, branchNames[1]);
    VERIFYJOB(j);
    j = m_plugin->switchBranch(baseUrl, branchNames[1]);
    VERIFYJOB(j);
    QVERIFY(writeFile(gitTest_BaseDir() + files[2], content));
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir() + files[2]));
    VERIFYJOB(j);
    j = m_plugin->commit(QStringLiteral("Committing to AnotherBranch"), QList<QUrl>() << baseUrl);
    VERIFYJOB(j);
    j = m_plugin->switchBranch(baseUrl, branchNames[0]);
    VERIFYJOB(j);
    j = m_plugin->mergeBranch(baseUrl, branchNames[1]);
    VERIFYJOB(j);
    qDebug() << j->errorString() ;

    jobLs = new DVcsJob(gitTest_BaseDir(), m_plugin);
    *jobLs << "git" << "ls-tree" << "--name-only" << "-r" << "HEAD" ;

    if (jobLs->exec() && jobLs->status() == KDevelop::VcsJob::JobSucceeded) {
        QStringList lsFiles = jobLs->output().split('\n');
        QVERIFY(lsFiles.contains(files[2]));
        qDebug() << "Files in this Branch: " << lsFiles;
    }

    j = m_plugin->switchBranch(baseUrl, QStringLiteral("master"));
    VERIFYJOB(j);
    j = m_plugin->mergeBranch(baseUrl, branchNames[1]);
    VERIFYJOB(j);
    qDebug() << j->errorString() ;

    jobLs = new DVcsJob(gitTest_BaseDir(), m_plugin);
    *jobLs << "git" << "ls-tree" << "--name-only" << "-r" << "HEAD" ;

    if (jobLs->exec() && jobLs->status() == KDevelop::VcsJob::JobSucceeded) {
        QStringList lsFiles = jobLs->output().split('\n');
        QVERIFY(lsFiles.contains(files[2]));
        qDebug() << "Files in this Branch: " << lsFiles;
    }

}

void GitInitTest::testBranching()
{
    repoInit();
    addFiles();
    commitFiles();

    const QUrl baseUrl = QUrl::fromLocalFile(gitTest_BaseDir());
    VcsJob* j = m_plugin->branches(baseUrl);
    VERIFYJOB(j);

    QString curBranch = runSynchronously(m_plugin->currentBranch(baseUrl)).toString();
    QCOMPARE(curBranch, QStringLiteral("master"));

    testBranch(QStringLiteral("new"));
    testBranch(QStringLiteral("averylongbranchnamejusttotestlongnames"));
    testBranch(QStringLiteral("KDE/4.10"));
}

void GitInitTest::revHistory()
{
    repoInit();
    addFiles();
    commitFiles();

    const QVector<KDevelop::DVcsEvent> commits = m_plugin->allCommits(gitTest_BaseDir());
    QVERIFY(!commits.isEmpty());
    QStringList logMessages;

    for (auto& commit : commits)
        logMessages << commit.log();

    QCOMPARE(commits.count(), 2);

    QCOMPARE(logMessages[0], QStringLiteral("KDevelop's Test commit2"));  //0 is later than 1!

    QCOMPARE(logMessages[1], QStringLiteral("Test commit"));

    QVERIFY(commits[1].parents().isEmpty());  //0 is later than 1!

    QVERIFY(!commits[0].parents().isEmpty()); //initial commit is on the top

    static const QRegularExpression commitSha1HashRegex("^[[:alnum:]]{40}$");
    QVERIFY(commits[1].commit().contains(commitSha1HashRegex));
    QVERIFY(commits[0].commit().contains(commitSha1HashRegex));
    QVERIFY(commits[0].parents()[0].contains(commitSha1HashRegex));
}

void GitInitTest::testAnnotation()
{
    repoInit();
    addFiles();
    commitFiles();

    // called after commitFiles
    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName(), QStringLiteral("An appended line"), QIODevice::Append));

    VcsJob* j = m_plugin->commit(QStringLiteral("KDevelop's Test commit3"), QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    j = m_plugin->annotate(QUrl::fromLocalFile(gitTest_BaseDir() + gitTest_FileName()), VcsRevision::createSpecialRevision(VcsRevision::Head));
    VERIFYJOB(j);

    QList<QVariant> results = j->fetchResults().toList();
    QCOMPARE(results.size(), 2);
    QVERIFY(results.at(0).canConvert<VcsAnnotationLine>());
    VcsAnnotationLine annotation = results.at(0).value<VcsAnnotationLine>();
    QCOMPARE(annotation.lineNumber(), 0);
    QCOMPARE(annotation.commitMessage(), QStringLiteral("KDevelop's Test commit2"));

    QVERIFY(results.at(1).canConvert<VcsAnnotationLine>());
    annotation = results.at(1).value<VcsAnnotationLine>();
    QCOMPARE(annotation.lineNumber(), 1);
    QCOMPARE(annotation.commitMessage(), QStringLiteral("KDevelop's Test commit3"));
}

void GitInitTest::testRemoveEmptyFolder()
{
    repoInit();

    QDir d(gitTest_BaseDir());
    d.mkdir(QStringLiteral("emptydir"));

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()+"emptydir/"));
    if (j) VERIFYJOB(j);

    QVERIFY(!d.exists(QStringLiteral("emptydir")));
}

void GitInitTest::testRemoveEmptyFolderInFolder()
{
    repoInit();

    QDir d(gitTest_BaseDir());
    d.mkdir(QStringLiteral("dir"));

    QDir d2(gitTest_BaseDir()+"dir");
    d2.mkdir(QStringLiteral("emptydir"));

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()+"dir/"));
    if (j) VERIFYJOB(j);

    QVERIFY(!d.exists(QStringLiteral("dir")));
}

void GitInitTest::testRemoveUnindexedFile()
{
    repoInit();

    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName(), QStringLiteral("An appended line"), QIODevice::Append));

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir() + gitTest_FileName()));
    if (j) VERIFYJOB(j);

    QVERIFY(!QFile::exists(gitTest_BaseDir() + gitTest_FileName()));
}

void GitInitTest::testRemoveFolderContainingUnversionedFiles()
{
    repoInit();

    QDir d(gitTest_BaseDir());
    d.mkdir(QStringLiteral("dir"));

    QVERIFY(writeFile(gitTest_BaseDir() + "dir/foo", QStringLiteral("An appended line"), QIODevice::Append));
    VcsJob* j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()+"dir"));
    VERIFYJOB(j);
    j = m_plugin->commit(QStringLiteral("initial commit"), QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir()));
    VERIFYJOB(j);

    QVERIFY(writeFile(gitTest_BaseDir() + "dir/bar", QStringLiteral("An appended line"), QIODevice::Append));

    j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir() + "dir"));
    if (j) VERIFYJOB(j);

    QVERIFY(!QFile::exists(gitTest_BaseDir() + "dir"));

}


void GitInitTest::removeTempDirs()
{
    const auto dirPath = gitTest_BaseDir();
    QDir dir(dirPath);
    if (dir.exists() && !dir.removeRecursively()) {
        qDebug() << "QDir::removeRecursively(" << dirPath << ") returned false";
    }
}

void GitInitTest::testDiff()
{
    repoInit();
    addFiles();
    commitFiles();

    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName(), QStringLiteral("something else")));

    VcsRevision srcrev = VcsRevision::createSpecialRevision(VcsRevision::Base);
    VcsRevision dstrev = VcsRevision::createSpecialRevision(VcsRevision::Working);
    VcsJob* j = m_plugin->diff(QUrl::fromLocalFile(gitTest_BaseDir()), srcrev, dstrev, IBasicVersionControl::Recursive);
    VERIFYJOB(j);

    KDevelop::VcsDiff d = j->fetchResults().value<KDevelop::VcsDiff>();
    QVERIFY(d.baseDiff().isLocalFile());
    QString path = d.baseDiff().toLocalFile();
    QVERIFY(QDir().exists(path+"/.git"));
}

void GitInitTest::testStash()
{
    repoInit();
    addFiles();
    commitFiles();
    const QVector<KDevelop::DVcsEvent> commits = m_plugin->allCommits(gitTest_BaseDir());
    QVERIFY(!commits.isEmpty());
    QStringList logMessages;

    for (auto& commit : commits)
        logMessages << commit.log();

    auto repo_url = QUrl::fromLocalFile(gitTest_BaseDir());

    // No stash
    QVERIFY(!m_plugin->hasStashes(gitTest_BaseDir()));
    VcsJob* j = m_plugin->stashList(gitTest_BaseDir());
    VERIFYJOB(j);
    QVERIFY(j->fetchResults().value<QList<GitPlugin::StashItem>>().isEmpty());

    // Do some changes
    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName(), QStringLiteral("NEW CONTENT TO STASH")));
    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName2(), QStringLiteral("No, REALLY NEW FOOO()!")));

    // Verify stashing works
    j = m_plugin->gitStash(gitTest_BaseDir(), {}, OutputJob::Silent);
    VERIFYJOB(j);
    QVERIFY(m_plugin->hasStashes(gitTest_BaseDir()));

    // Verify stash list gives us the single stash
    j = m_plugin->stashList(gitTest_BaseDir());
    VERIFYJOB(j);
    auto items = j->fetchResults().value<QList<GitPlugin::StashItem>>();
    QVERIFY(items.length() == 1);
    QVERIFY(items[0].stackDepth == 0);
    QVERIFY(items[0].branch == runSynchronously(m_plugin->currentBranch(repo_url)).toString());
    QVERIFY(commits[0].commit() == items[0].parentSHA);

    // Verify stash on new branch gives us correct parent branch
    //
    // 1. switch to a new branch
    auto rev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Head);
    j = m_plugin->branch(repo_url, rev, QStringLiteral("my-new-branch"));
    VERIFYJOB(j);
    j = m_plugin->switchBranch(repo_url, QStringLiteral("my-new-branch"));
    VERIFYJOB(j);

    // 2. Do more changes & stash them
    QString testFileContent = QStringLiteral("2. NEW CONTENT TO STASH");
    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName(), testFileContent));
    QVERIFY(writeFile(gitTest_BaseDir() + gitTest_FileName2(), QStringLiteral("2. No, REALLY NEW FOOO()!")));
    j = m_plugin->gitStash(gitTest_BaseDir(), {}, OutputJob::Silent);
    VERIFYJOB(j);
    QFile testFile(gitTest_BaseDir() + gitTest_FileName());
    testFile.open(QIODevice::ReadOnly);
    auto testFileContentAfterStash = QString::fromUtf8(testFile.readAll());
    testFile.close();
    QVERIFY(testFileContent != testFileContentAfterStash);


    // 3. Verify stash list gives us a new stash
    j = m_plugin->stashList(gitTest_BaseDir());
    VERIFYJOB(j);
    items = j->fetchResults().value<QList<GitPlugin::StashItem>>();
    QVERIFY(items.length() == 2);

    // 4. Find the newest stash
    bool found = false;
    for(const auto& item : items) {
        if (item.stackDepth != 0) continue;
        // 5. Verify that the new branch is parent of the new stash
        QVERIFY(item.branch == QStringLiteral("my-new-branch"));
        found = true;
    }
    QVERIFY(found);

    // Verify that stash pop recreates the contents and drops the stash item
    j = m_plugin->gitStash(gitTest_BaseDir(), {QStringLiteral("pop")}, OutputJob::Silent);
    VERIFYJOB(j);
    testFile.open(QIODevice::ReadOnly);
    auto testFileContentAfterPop = QString::fromUtf8(testFile.readAll());
    QVERIFY(testFileContentAfterPop == testFileContent);
    j = m_plugin->stashList(gitTest_BaseDir());
    VERIFYJOB(j);
    items = j->fetchResults().value<QList<GitPlugin::StashItem>>();
    QVERIFY(items.length() == 1);

}

void GitInitTest::testRegisterRepositoryForCurrentBranchChanges()
{
    repoInit();
    addFiles();
    commitFiles();

    // Remove the trailing slash so that the QCOMPARE(X, baseUrl) calls below succeed.
    const auto baseUrl = QUrl::fromLocalFile(gitTest_BaseDir().chopped(1));

    QSignalSpy branchChangedSpy(m_plugin, &GitPlugin::repositoryBranchChanged);

    const auto events = runSynchronously(m_plugin->log(baseUrl, VcsRevision::createSpecialRevision(VcsRevision::Base),
                                                       VcsRevision::createSpecialRevision(VcsRevision::Start)))
                            .toList();
    QCOMPARE(events.size(), 2); // commitFiles() creates two commits
    std::vector<VcsRevision> revisions;
    revisions.reserve(events.size());
    std::transform(events.cbegin(), events.cend(), std::back_inserter(revisions), [](const QVariant& event) {
        return event.value<VcsEvent>().revision();
    });

    const auto master = QStringLiteral("master");
    const auto branch1 = QStringLiteral("branch1");
    const auto branch2 = QStringLiteral("branch2");
    // The master branch already exists, create two more branches.
    for (const auto* const branch : {&branch1, &branch2}) {
        const auto& revision = branch == &branch1 ? revisions.front() : revisions.back();
        auto* const job = m_plugin->branch(baseUrl, revision, *branch);
        VERIFYJOB(job);
    }

#define SWITCH_BRANCH(branchName)                                                                                      \
    do {                                                                                                               \
        auto* const job = m_plugin->switchBranch(baseUrl, branchName);                                                 \
        VERIFYJOB(job);                                                                                                \
    } while (false)

    // The signal GitPlugin::repositoryBranchChanged() is emitted
    // after a delay of one second. Wait two seconds to be safe.
    constexpr auto signalWaitInterval = 2000;

    const auto verifyZeroSignalsEvenAfterWait = [&branchChangedSpy] {
        QCOMPARE(branchChangedSpy.count(), 0);
        QTest::qWait(signalWaitInterval);
        QCOMPARE(branchChangedSpy.count(), 0);
    };

    const auto verifyAndRemoveSingleSignalAfterWait = [&branchChangedSpy, &baseUrl] {
        QCOMPARE(branchChangedSpy.count(), 0); // the signal is emitted after a delay
        QTest::qWait(signalWaitInterval);
        QCOMPARE(branchChangedSpy.count(), 1);
        QCOMPARE(branchChangedSpy.takeFirst().constFirst().toUrl(), baseUrl); // verify and remove the signal
        QCOMPARE(branchChangedSpy.count(), 0);
    };

#define VERIFY_ZERO_SIGNALS_EVEN_AFTER_WAIT()                                                                          \
    do {                                                                                                               \
        verifyZeroSignalsEvenAfterWait();                                                                              \
        RETURN_IF_TEST_FAILED();                                                                                       \
    } while (false)

#define VERIFY_AND_REMOVE_SINGLE_SIGNAL_AFTER_WAIT()                                                                   \
    do {                                                                                                               \
        verifyAndRemoveSingleSignalAfterWait();                                                                        \
        RETURN_IF_TEST_FAILED();                                                                                       \
    } while (false)

    std::array<QObject, 3> listeners;

    // No listeners have been registered => no signal.
    SWITCH_BRANCH(branch1);
    VERIFY_ZERO_SIGNALS_EVEN_AFTER_WAIT();

    m_plugin->registerRepositoryForCurrentBranchChanges(baseUrl, &listeners[0]);

    // Registered a listener => signal.
    SWITCH_BRANCH(master);
    VERIFY_AND_REMOVE_SINGLE_SIGNAL_AFTER_WAIT();

    // A single signal is emitted for multiple close-by current branch changes due to an optimizing compression.
    for (auto i = 0; i < 5; ++i) {
        for (const auto* const branch : {&branch1, &branch2, &master}) {
            SWITCH_BRANCH(*branch);
        }
    }
    VERIFY_AND_REMOVE_SINGLE_SIGNAL_AFTER_WAIT();

    m_plugin->registerRepositoryForCurrentBranchChanges(baseUrl, &listeners[1]);

    // A single signal is emitted for any number of listeners.
    SWITCH_BRANCH(branch2);
    VERIFY_AND_REMOVE_SINGLE_SIGNAL_AFTER_WAIT();

    m_plugin->unregisterRepositoryForCurrentBranchChanges(baseUrl, &listeners[0]);

    // The signal is emitted because one listener still remains.
    SWITCH_BRANCH(revisions.front().prettyValue()); // test checking out a revision instead of a branch
    VERIFY_AND_REMOVE_SINGLE_SIGNAL_AFTER_WAIT();

    m_plugin->unregisterRepositoryForCurrentBranchChanges(baseUrl, &listeners[0]);

    // The signal is emitted because unregistering a listener, even redundantly, never affects other listeners.
    runCommand("git", {"checkout", master}); // test checking out externally, bypassing GitPlugin
    VERIFY_AND_REMOVE_SINGLE_SIGNAL_AFTER_WAIT();

    m_plugin->unregisterRepositoryForCurrentBranchChanges(baseUrl, &listeners[1]);

    // Unregistered all listeners => no signal anymore.
    SWITCH_BRANCH(branch2);
    VERIFY_ZERO_SIGNALS_EVEN_AFTER_WAIT();

    m_plugin->registerRepositoryForCurrentBranchChanges(baseUrl, &listeners[2]);

    // Registered a listener again => the signal is emitted.
    SWITCH_BRANCH(branch1);
    VERIFY_AND_REMOVE_SINGLE_SIGNAL_AFTER_WAIT();

    m_plugin->unregisterRepositoryForCurrentBranchChanges(baseUrl, &listeners[2]);
}

QTEST_MAIN(GitInitTest)

// #include "gittest.moc"
#include "moc_test_git.cpp"
