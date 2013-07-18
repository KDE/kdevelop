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

#ifndef KDEVPLATFORM_VCSBLACKBOXTEST_H
#define KDEVPLATFORM_VCSBLACKBOXTEST_H

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <vector>
#include <vcs/vcsstatusinfo.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

class KTempDir;

class KProcess;

namespace KDevelop
{

class TestCore;

class IDistributedVersionControl;

class ICentralizedVersionControl;

class VcsJob;
}

class noncopyable
{
protected:
    noncopyable() {}

    ~noncopyable() {}

private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};


class Repo
            : noncopyable
{
public:
    typedef KTempDir* WorkDirPtr;
    Repo(KDevelop::IBasicVersionControl* _vcs, bool distributed);

    KDevelop::IBasicVersionControl* vcs;
    bool isDistributed;
    WorkDirPtr workingDir;
    KUrl rootUrl;
    typedef QMap<KUrl, KDevelop::VcsStatusInfo> StateMap;
    StateMap state;

    void add(int line, const KUrl::List & objects, KDevelop::IBasicVersionControl::RecursionMode mode);
    void commit(int line, const QString & message, const KUrl::List & objects, KDevelop::IBasicVersionControl::RecursionMode mode);
    KUrl::List objects() const;
    void verifyState(int line) const;
};

class CRepo
            : public Repo
{
public:
    CRepo(KDevelop::ICentralizedVersionControl* _cvcs, const KDevelop::VcsLocation & _repositoryLocation, const WorkDirPtr & _repositoryDir);
    KDevelop::ICentralizedVersionControl* cvcs;
    KDevelop::VcsLocation repositoryLocation;
    WorkDirPtr repositoryDir;
};

class DRepo
            : public Repo
{
public:
    explicit DRepo(KDevelop::IDistributedVersionControl* _dvcs);
    KDevelop::IDistributedVersionControl* dvcs;
};

class VcsBlackBoxTest
            : public QObject
{
    Q_OBJECT
private:
    void repoInit(DRepo & r, DRepo & s);
    void repoInit(CRepo & r, CRepo & s);
    void testAddRevert(Repo & r);
    void testCommitModifyRevert(Repo & r);
    void testSharedOps(DRepo & r, DRepo & s);
    void testSharedOps(CRepo & r, CRepo & s);

private slots:
    void initTestCase();
    void testInit();
    void testAddRevert();
    void testCommitModifyRevert();
    void testSharedOps();
    void cleanupTestCase();

private:
    typedef Repo* RepoPtr;
    QList<RepoPtr> m_primary;
    QList<RepoPtr> m_secondary;
    void removeTempDirs();
};

#endif
