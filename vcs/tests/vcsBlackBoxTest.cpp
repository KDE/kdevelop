/***************************************************************************
 *   This file was taken from KDevelop's git plugin                        *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Generalised black box test for IBasicVersionControl and derived ones  *
 *   Copyright 2009 Fabian Wiesel <fabian.wiesel@googlemail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "vcsBlackBoxTest.h"

#include <QtTest/QtTest>
#include <qtest_kde.h>
#include <KTempDir>
#include <kdebug.h>
#include <kparts/part.h>
#include <kio/netaccess.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <vcs/interfaces/idistributedversioncontrol.h>
#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcsjob.h>
#include <vcs/dvcs/dvcsjob.h>
#include <plugins/cvs/cvsjob.h>
#include <iostream>

#define PATHETIC    // A little motivator to make things work right :)
#if defined(PATHETIC)
const QString vcsTestDir0("testdir0");
const QString vcsTestDir1("testdir1");
const QString vcsTest_FileName0("foo");
const QString vcsTest_FileName1("bar");
const QString keywordText("text");
#else
const QString vcsTestDir0("dvcs\t testdir");   // Directory containing whitespaces
const QString vcsTestDir1("--help");           // Starting with hyphen for command-line tools
const QString vcsTest_FileName0("foo\t bar");
const QString vcsTest_FileName1("--help");
const QString keywordText("Author:\nDate:\nCommit:\n------------------------------------------------------------------------\nr999999 | ehrman | 1989-11-09 18:53:00 +0100 (Thu, 09 Nov 1989) | 1 lines\nthe line\n");  // Text containing keywords of the various vcs-programs
#endif

const QString simpleText("It's foo!\n");
const QString simpleAltText("No, foo()! It's bar()!\n");

// #define VERBOSE
#if defined(VERBOSE)
#define TRACE(X) kDebug() << X
#else
#define TRACE(X) { line = line; }
#endif

using namespace KDevelop;

void validatingExecJob(int line, VcsJob* j, VcsJob::JobStatus status = VcsJob::JobSucceeded)
{
    TRACE("Called from line" << line);
    QVERIFY(j);
    // Print the commands in full, for easier bug location
#if 0
    if (QLatin1String(j->metaObject()->className()) == "DVcsJob") {
        kDebug() << "Command: \"" << ((DVcsJob*)j)->process()->program() << ((DVcsJob*)j)->process()->workingDirectory();
        kDebug() << "Output: \"" << ((DVcsJob*)j)->output();
    }
#endif

    if (!j->exec()) {
        kDebug() << j->errorString();
#if 1
        // On error, wait for key in order to allow manual state inspection
        char c;
        std::cin.read(&c, 1);
#endif
    }

    QCOMPARE(j->status(), status);
}

void readVerify(int line, VcsStatusInfo const & status, QString const & contents)
{
    TRACE("Called from line" << line);
    QFile f(status.url().toLocalFile());
    QVERIFY(f.open(QIODevice::ReadOnly));
    QTextStream filecontents(&f);
    QCOMPARE(filecontents.readAll(), contents);
}

void verifiedWrite(int line, VcsStatusInfo & status, QString const & contents)
{
    TRACE("Called from line" << line);
    QFile f(status.url().toLocalFile());
    QVERIFY(f.open(QIODevice::WriteOnly));
    QTextStream filecontents(&f);
    filecontents << contents;
    filecontents.flush();
    f.flush();
    status.setState(VcsStatusInfo::ItemModified);
}

void initialVerifiedWrite(int line, VcsStatusInfo & status, QString const & contents)
{
    verifiedWrite(line, status, contents);
    status.setState(VcsStatusInfo::ItemUnknown);
}

void fetchStatus(int line, QList<VcsStatusInfo> & statuslist, IBasicVersionControl * vcs, KUrl::List const & list, IBasicVersionControl::RecursionMode recursion)
{
    TRACE("Called from line" << line);
    VcsJob* j = vcs->status(list, recursion);
    QVERIFY(j);
    validatingExecJob(line, j);
    QVariant untyped = j->fetchResults();
    QVERIFY(untyped.canConvert(QVariant::List));
    QList<QVariant> untypedList = untyped.toList();

    statuslist.clear();
    foreach(const QVariant & v, untypedList) {
        QVERIFY(v.canConvert<VcsStatusInfo>());
        statuslist.push_back(qVariantValue<VcsStatusInfo>(v));
    }
}

void fileStatus(int line, VcsStatusInfo & status, IBasicVersionControl * vcs, KUrl const & file)
{
    QList<VcsStatusInfo> statuslist;
    fetchStatus(line, statuslist, vcs, KUrl::List(file), IBasicVersionControl::NonRecursive);
    QCOMPARE(statuslist.size(), 1);
    // In case, we don't have a single status, despite the request, we only want one error
    foreach(const VcsStatusInfo & s, statuslist) {
        if (s.url() == file) {
            status = s;
            return;
        }
    }

    // In case, we don't find the file, assume it is the first
    status = statuslist.front();
    QCOMPARE(status.url(), file); // Fails and provides the corresponding fmessage
}


Repo::Repo(IBasicVersionControl* _vcs, bool distributed)
        : vcs(_vcs), isDistributed(distributed), workingDir(new KTempDir()), rootUrl(workingDir->name())
{}

void Repo::add(int line, KUrl::List const & objects, IBasicVersionControl::RecursionMode mode)
{
    TRACE("Called from line" << line);
    KUrl::List objects0 = objects;
    KUrl::List objects2;

    if (mode == IBasicVersionControl::Recursive && vcs->name() == "Subversion") {
        // SVN fails adding recursively files from the root, so we fake it
        objects0.clear();
        foreach(const VcsStatusInfo & i, state) {
            objects0.push_back(i.url());
        }
    }

    foreach(const KUrl & o, objects0) {

        QFileInfo fi(o.toLocalFile());

        if (fi.isFile()) {
            KUrl parentDir = KUrl(fi.absolutePath());

            while (!vcs->isVersionControlled(parentDir)) {
                QVERIFY(!parentDir.isParentOf(rootUrl));
                objects2.push_front(parentDir);
                parentDir = parentDir.upUrl();
            }

            state[o].setState(VcsStatusInfo::ItemAdded);
        }

        objects2.push_back(o);
    }

    if (mode == IBasicVersionControl::Recursive) {
        QVERIFY(objects.size() == 1 && objects.front() == rootUrl); // The only case we use in the test

        for (Repo::StateMap::iterator i = state.begin(); i != state.end(); ++i) {
            i->setState(VcsStatusInfo::ItemAdded);
        }
    }

    TRACE("Adding: " << objects2); //  << " to " << rootUrl;

    validatingExecJob(__LINE__, vcs->add(objects2, mode));
    verifyState(line);
}


void Repo::commit(int line, const QString& message, const KUrl::List& objects, KDevelop::IBasicVersionControl::RecursionMode mode)
{
    TRACE("Called from line" << line);
    KUrl::List objects2;
    foreach(const KUrl & o, objects) {
        QFileInfo fi(o.toLocalFile());

        if (fi.isFile()) {
            // We commit all the parent directories, too
            KUrl parentDir = KUrl(fi.absolutePath());

            while (rootUrl != parentDir && vcs->isVersionControlled(parentDir)) {
                QVERIFY(!parentDir.isParentOf(rootUrl));
                objects2.push_front(parentDir);
                parentDir = parentDir.upUrl();
            }

            state[o].setState(VcsStatusInfo::ItemUpToDate);
        }

        objects2.push_back(o);
    }

    if (mode == IBasicVersionControl::Recursive) {
        QVERIFY(objects.size() == 1 && objects.front() == rootUrl); // The only case we use in the test

        for (Repo::StateMap::iterator i = state.begin(); i != state.end(); ++i) {
            i->setState(VcsStatusInfo::ItemUpToDate);
        }
    }

    TRACE("Commiting: " << objects);

    validatingExecJob(__LINE__, vcs->commit(message, objects2, mode));
    verifyState(line);
}

// TODO: Randomise batch-size and order of isVersionControlled() status()-calls
void Repo::verifyState(int line) const
{
    TRACE("Called from line" << line);

    for (StateMap::const_iterator i = state.begin(); i != state.end(); ++i) {
        bool expectedVersionControlledStatus = (i->state() != VcsStatusInfo::ItemUnknown);
        QCOMPARE(vcs->isVersionControlled(i->url()), expectedVersionControlledStatus);

        if (expectedVersionControlledStatus) {  // fileStatus() may fail, when not version controlled
            VcsStatusInfo status;
            fileStatus(line, status, vcs, i->url());

            if (status != *i) {
                kDebug() << status.url() << ':' << status.state() << " != " << i->state();
            }

            QCOMPARE(status, *i);
        }
    }
}

KUrl::List Repo::objects() const
{
    KUrl::List list;
    foreach(const VcsStatusInfo & info, state) {
        list.push_back(info.url());
    }

    return list;
}

CRepo::CRepo(ICentralizedVersionControl* _cvcs, VcsLocation const & _repositoryLocation,  WorkDirPtr const & _repositoryDir)
        : Repo(_cvcs, false)
        , cvcs(_cvcs)
        , repositoryLocation(_repositoryLocation)
        , repositoryDir(_repositoryDir)
{}

DRepo::DRepo(IDistributedVersionControl* _dvcs)
        : Repo(_dvcs, true)
        , dvcs(_dvcs)
{}

void fillWorkingDirectory(Repo & r)
{
    //we start it after repoInit, so we still have empty dvcs repo
    {
        QDir dir(r.workingDir->name());
        QVERIFY(dir.mkdir(vcsTestDir0));
        QVERIFY(dir.cd(vcsTestDir0));
        KUrl file0(dir.absoluteFilePath(vcsTest_FileName0));
        VcsStatusInfo status;
        status.setUrl(file0);
        status.setState(VcsStatusInfo::ItemUnknown);
        r.state[file0] = status;
    }

    {
        QDir dir(r.workingDir->name());
        QVERIFY(dir.mkdir(vcsTestDir1));
        QVERIFY(dir.cd(vcsTestDir1));
        KUrl file1(dir.absoluteFilePath(vcsTest_FileName1));
        VcsStatusInfo status;
        status.setUrl(file1);
        status.setState(VcsStatusInfo::ItemUnknown);
        r.state[file1] = status;
    }

    KUrl::List o = r.objects();

    initialVerifiedWrite(__LINE__, r.state[o[0]], simpleText);
    initialVerifiedWrite(__LINE__, r.state[o[1]], keywordText);
}

void VcsBlackBoxTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::Default);
    IPluginController *pluginController = Core::self()->pluginController();
    QList<IPlugin*> plugins = pluginController ->allPluginsForExtension("org.kdevelop.IBasicVersionControl");

    kDebug() << "Plugins: " << plugins.size();
    foreach(IPlugin* p,  plugins) {
        IDistributedVersionControl* idistr = p->extension<IDistributedVersionControl>();

        if (idistr) {
            kDebug() << "Component: \"" << p->componentData().componentName() << "\" Name: \"" << idistr->name() << '"';
            QVERIFY2(!idistr->name().isEmpty(), "IBasicVersionControl may not return an empty name");

            m_primary.push_back(RepoPtr(new DRepo(idistr)));

            m_secondary.push_back(RepoPtr(new DRepo(idistr)));
        }
        else {
            ICentralizedVersionControl* icentr = p->extension<ICentralizedVersionControl>();
            QVERIFY(icentr);
            Repo::WorkDirPtr remoteRepos(new KTempDir());
            kDebug() << "Component: \"" << p->componentData().componentName() << "\" Name: \"" << icentr->name() << '"';
            QVERIFY2(!icentr->name().isEmpty(), "IBasicVersionControl must return a non-empty name");

            if (icentr->name() == "Subversion") {
                KProcess cmd;
                cmd.setWorkingDirectory(remoteRepos->name());
                cmd << "svnadmin" << "create" << ".";
                QCOMPARE(cmd.execute(10000), 0);
                VcsLocation repositoryLocation;
                repositoryLocation.setRepositoryServer("file://" + remoteRepos->name());
                m_primary.push_back(RepoPtr(new CRepo(icentr, repositoryLocation, remoteRepos)));
                m_secondary.push_back(RepoPtr(new CRepo(icentr, repositoryLocation, remoteRepos)));
            }
            else if (icentr->name() == "CVS") {
                static const QString repoSubDirName("repo");
                static const QString emptyImportDirName("empty");
                QDir repodir(remoteRepos->name());
                repodir.mkdir(repoSubDirName);
                repodir.mkdir(emptyImportDirName);
                VcsLocation repositoryLocation;
                repositoryLocation.setRepositoryServer(remoteRepos->name() + repoSubDirName);
                repositoryLocation.setRepositoryModule("testmodule");
                KProcess cmd;
                cmd.setWorkingDirectory(remoteRepos->name() + emptyImportDirName);
                cmd << "cvs" << "-d" << repositoryLocation.repositoryServer() << "init";
                QCOMPARE(cmd.execute(10000), 0);
                repositoryLocation.setRepositoryTag("start");
                repositoryLocation.setUserData(qVariantFromValue(QString("vcsBlackBoxTest")));
                KUrl emptySourcedir(remoteRepos->name() + emptyImportDirName);
                validatingExecJob(__LINE__, icentr->import("Inital import", emptySourcedir, repositoryLocation));
                repositoryLocation.setUserData(QVariant());
                repositoryLocation.setRepositoryTag(QString());
                m_primary.push_back(RepoPtr(new CRepo(icentr, repositoryLocation, remoteRepos)));
                m_secondary.push_back(RepoPtr(new CRepo(icentr, repositoryLocation, remoteRepos)));
            }
        }
    }
}

void VcsBlackBoxTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void VcsBlackBoxTest::repoInit(DRepo& r, DRepo& s)
{
    kDebug() << r.vcs->name();

    KUrl repo0(r.workingDir->name());
    KUrl repo1(s.workingDir->name());

    // Pre-Init: Non-version controlled directories
    QVERIFY(!r.vcs->isVersionControlled(repo0));
    QVERIFY(!s.vcs->isVersionControlled(repo1));

    // make job that creates the local repository

    validatingExecJob(__LINE__, r.dvcs->init(repo0));

    // Post-Init: First sub-directory is version-controlled
    QVERIFY(r.vcs->isVersionControlled(repo0));
    QVERIFY(!s.vcs->isVersionControlled(repo1));

    validatingExecJob(__LINE__, s.dvcs->init(repo1));

    // Post-Init: Second sub-directory is also version-controlled
    QVERIFY(r.vcs->isVersionControlled(repo0));
    QVERIFY(s.vcs->isVersionControlled(repo1));

    // Delete the second repository
    QVERIFY(KIO::NetAccess::del(repo1, 0));
    QDir dir;
    dir.mkpath(s.workingDir->name());

    // Only the first should still be version-controlled
    QVERIFY(r.vcs->isVersionControlled(repo0));
    QVERIFY(!s.vcs->isVersionControlled(repo1));
}

void VcsBlackBoxTest::repoInit(CRepo& r, CRepo& s)
{
    kDebug() << r.vcs->name();

    KUrl wd0(r.workingDir->name());
    KUrl wd1(s.workingDir->name());

    QVERIFY(!r.vcs->isVersionControlled(wd0));
    QVERIFY(!s.vcs->isVersionControlled(wd1));
    
    validatingExecJob(__LINE__, r.cvcs->createWorkingCopy(r.repositoryLocation, wd0, IBasicVersionControl::Recursive));

    QVERIFY(r.vcs->isVersionControlled(wd0));
    QVERIFY(!s.vcs->isVersionControlled(wd1));

    validatingExecJob(__LINE__, s.cvcs->createWorkingCopy(s.repositoryLocation, wd1, IBasicVersionControl::Recursive));

    QVERIFY(r.vcs->isVersionControlled(wd0));
    QVERIFY(s.vcs->isVersionControlled(wd1));

    // Delete the second working-directory
    QVERIFY(KIO::NetAccess::del(wd1, 0));
    QDir dir;
    dir.mkpath(s.workingDir->name());

    // Only the first should still be version-controlled
    QVERIFY(r.vcs->isVersionControlled(wd0));
    QVERIFY(!s.vcs->isVersionControlled(wd1));
}

void VcsBlackBoxTest::testAddRevert(Repo & r)
{
    IBasicVersionControl * vcs = r.vcs;
    kDebug() << vcs->name();
    KUrl workingDir(r.workingDir->name());

    fillWorkingDirectory(r);

    r.verifyState(__LINE__);

    // TODO: Define and check the behaviour of a non-recursive status-query on a directory
    {
        // One with
        workingDir.adjustPath(KUrl::RemoveTrailingSlash);
        QList<VcsStatusInfo> resultsWithoutTrailingSlash;
        fetchStatus(__LINE__, resultsWithoutTrailingSlash, vcs, KUrl::List(workingDir), IBasicVersionControl::Recursive);
        QVERIFY(2 == resultsWithoutTrailingSlash.size() || 3 == resultsWithoutTrailingSlash.size());

        // One without
        workingDir.adjustPath(KUrl::AddTrailingSlash);
        QList<VcsStatusInfo> resultsWithTrailingSlash;
        fetchStatus(__LINE__, resultsWithTrailingSlash, vcs, KUrl::List(workingDir), IBasicVersionControl::Recursive);

        // TODO: As there is no constraint on the order of the results, they may be reordered and the lists may differ
        QCOMPARE(resultsWithoutTrailingSlash, resultsWithTrailingSlash);
    }

    // Adding sequentially

    for (Repo::StateMap::const_iterator i = r.state.constBegin(); i != r.state.constEnd(); ++i) {
        r.add(__LINE__, KUrl::List(i->url()), IBasicVersionControl::NonRecursive);
    }

    // Reverting the add in the same order as they were added
    for (Repo::StateMap::iterator i = r.state.begin(); i != r.state.end(); ++i) {
        validatingExecJob(__LINE__, vcs->revert(KUrl::List(i->url())));
        i->setState(VcsStatusInfo::ItemUnknown);
        r.verifyState(__LINE__);
    }

    typedef std::reverse_iterator<Repo::StateMap::const_iterator> StateMapReverseIterator;

    // Adding sequentially; this time in reverse order
    StateMapReverseIterator rbegin(r.state.constEnd()), rend(r.state.constBegin());

    for (StateMapReverseIterator i = rbegin; i != rend; ++i) {
        r.add(__LINE__, KUrl::List(i->url()), IBasicVersionControl::NonRecursive);
    }

    // Reverting the add in different order
    for (Repo::StateMap::iterator i = r.state.begin(); i != r.state.end(); ++i) {
        validatingExecJob(__LINE__, vcs->revert(KUrl::List(i->url())));
        i->setState(VcsStatusInfo::ItemUnknown);
        r.verifyState(__LINE__);
    }

    // Back to the beginning, now batch add
    r.add(__LINE__, KUrl::List(r.rootUrl), IBasicVersionControl::Recursive);

    // Batch revert
    validatingExecJob(__LINE__, vcs->revert(r.objects()));

    for (Repo::StateMap::iterator i = r.state.begin(); i != r.state.end(); ++i) {
        i->setState(VcsStatusInfo::ItemUnknown);
    }

    r.verifyState(__LINE__);
}

void VcsBlackBoxTest::testCommitModifyRevert(Repo & r)
{
    kDebug() << r.vcs->name();
    KUrl::List objects = r.objects();
    readVerify(__LINE__, r.state[objects[0]], simpleText);
    readVerify(__LINE__, r.state[objects[1]], keywordText);
    // We now have some files, some of which are not versioned, first add them all
    r.add(__LINE__, r.rootUrl, IBasicVersionControl::Recursive);

    // First, commit just one file
    r.commit(__LINE__, QString(keywordText), KUrl::List(objects[0]), IBasicVersionControl::NonRecursive);
    readVerify(__LINE__, r.state[objects[0]], simpleText);
    readVerify(__LINE__, r.state[objects[1]], keywordText);

    // TODO: Behaviour on commit with empty commit messages is not defined by the interface

    // Next, the other file with a possibly conflicting message
    r.commit(__LINE__, QString("--help"), KUrl::List(objects[1]), IBasicVersionControl::NonRecursive);

    // Modify the file
    verifiedWrite(__LINE__, r.state[objects[0]], simpleAltText);
    r.state[objects[0]].setState(VcsStatusInfo::ItemModified);
    r.verifyState(__LINE__);
    readVerify(__LINE__, r.state[objects[0]], simpleAltText);

    // Try to revert the wrong file
    VcsJob * j = r.vcs->revert(KUrl::List(objects[1]), IBasicVersionControl::NonRecursive);

    if (j) {    // May refuse to do stupid things
        j->exec();
    }

    r.verifyState(__LINE__);

    readVerify(__LINE__, r.state[objects[0]], simpleAltText);
    readVerify(__LINE__, r.state[objects[1]], keywordText);

    // Now revert the correct file
    validatingExecJob(__LINE__, r.vcs->revert(KUrl::List(objects[0]), IBasicVersionControl::NonRecursive));
    r.state[objects[0]].setState(VcsStatusInfo::ItemUpToDate);
    r.verifyState(__LINE__);
    readVerify(__LINE__, r.state[objects[0]], simpleText);

    // Modify both files
    verifiedWrite(__LINE__, r.state[objects[0]], keywordText);
    verifiedWrite(__LINE__, r.state[objects[1]], simpleAltText);
    r.verifyState(__LINE__);

    // Revert recursively
    validatingExecJob(__LINE__, r.vcs->revert(KUrl::List(KUrl(r.workingDir->name())), IBasicVersionControl::Recursive));

    for (Repo::StateMap::iterator i = r.state.begin(); i != r.state.end(); ++i) {
        i->setState(VcsStatusInfo::ItemUpToDate);
    }

    r.verifyState(__LINE__);

    readVerify(__LINE__, r.state[objects[0]], simpleText);

    // Modify both files again
    verifiedWrite(__LINE__, r.state[objects[0]], keywordText);
    verifiedWrite(__LINE__, r.state[objects[1]], simpleAltText);
    r.verifyState(__LINE__);

    // Commit everything recursively
    r.commit(__LINE__, QString("Content Changes"), KUrl::List(KUrl(r.workingDir->name())), IBasicVersionControl::Recursive);
}

void VcsBlackBoxTest::testSharedOps(DRepo&, DRepo&)
{
}

void VcsBlackBoxTest::testSharedOps(CRepo&, CRepo&)
{
}

void VcsBlackBoxTest::testInit()
{
    for (int i = 0; i < m_primary.size(); ++i) {
        if (m_primary[i]->isDistributed) {
            DRepo * a = static_cast<DRepo*>(m_primary[i]);
            DRepo * b = static_cast<DRepo*>(m_secondary[i]);
            repoInit(*a, *b);
        }
        else {
            CRepo * a = static_cast<CRepo*>(m_primary[i]);
            CRepo * b = static_cast<CRepo*>(m_secondary[i]);
            repoInit(*a, *b);
        }
    }
}

void VcsBlackBoxTest::testAddRevert()
{
    for (int i = 0; i < m_primary.size(); ++i) {
        testAddRevert(*m_primary[i]);
    }
}

void VcsBlackBoxTest::testCommitModifyRevert()
{
    for (int i = 0; i < m_primary.size(); ++i) {
        testCommitModifyRevert(*m_primary[i]);
    }
}

void VcsBlackBoxTest::testSharedOps()
{
    for (int i = 0; i < m_primary.size(); ++i) {
        Repo * a = m_primary[i];
        Repo * b = m_secondary[i];

        if (m_primary[i]->isDistributed) {
            testSharedOps(*static_cast<DRepo*>(a), *static_cast<DRepo*>(b));
        }
        else {
            testSharedOps(*static_cast<CRepo*>(a), *static_cast<CRepo*>(b));
        }
    }
}

QTEST_KDEMAIN(VcsBlackBoxTest, GUI)

// #include "dvcstest.moc"
