/***************************************************************************
 *   This file was taken from KDevelop's git plugin                        *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Adapted for Mercurial                                                 *
 *   Copyright 2009 Fabian Wiesel <fabian.wiesel@fu-berlin.de>             *
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

#ifndef MERCURIAL_PLUGIN_H
#define MERCURIAL_PLUGIN_H

#include <vcs/interfaces/idistributedversioncontrol.h>
#include <vcs/dvcs/dvcsplugin.h>
#include <QtCore/QObject>
#include <vcs/vcsstatusinfo.h>

namespace KDevelop
{

class VcsJob;

class VcsRevision;
}

class MercurialExecutor;

/**
 * This is the main class of KDevelop's Mercurial plugin.
 *
 * It implements the DVCS dependent things not implemented in KDevelop::DistributedVersionControlPlugin
 * @author Fabian Wiesel <fabian.wiesel@fu-berlin.de>
 */

class MercurialPlugin
            : public KDevelop::DistributedVersionControlPlugin
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::IDistributedVersionControl)

    friend class MercurialExecutor;

public:
    MercurialPlugin(QObject *parent, const QVariantList & args = QVariantList());
    ~MercurialPlugin();

    bool isValidDirectory(const KUrl &dirPath);
    bool isVersionControlled(const KUrl &path);
    QString name() const;

    KDevelop::VcsJob* init(const KUrl & directory);
    KDevelop::VcsJob* clone(const KDevelop::VcsLocation & localOrRepoLocationSrc, const KUrl & repository);
    KDevelop::VcsJob* add(const KUrl::List& localLocations,
                 KDevelop::IBasicVersionControl::RecursionMode recursion  = KDevelop::IBasicVersionControl::Recursive);
    KDevelop::VcsJob* copy(const KUrl& localLocationSrc,
                  const KUrl& localLocationDst);
    KDevelop::VcsJob* move(const KUrl& localLocationSrc,
                  const KUrl& localLocationDst);

    KDevelop::VcsJob* commit(const QString& message,
                    const KUrl::List& localLocations,
                    KDevelop::IBasicVersionControl::RecursionMode recursion);
    KDevelop::VcsJob* diff(const KUrl& fileOrDirectory,
                  const KDevelop::VcsRevision & srcRevision,
                  const KDevelop::VcsRevision & dstRevision,
                  KDevelop::VcsDiff::Type diffType,
                  KDevelop::IBasicVersionControl::RecursionMode recursionMode);

    KDevelop::VcsJob* remove(const KUrl::List& files);
    KDevelop::VcsJob* status(const KUrl::List& localLocations,
                    KDevelop::IBasicVersionControl::RecursionMode recursion);
    KDevelop::VcsJob* revert(const KUrl::List& localLocations,
                    KDevelop::IBasicVersionControl::RecursionMode recursion);

    KDevelop::VcsJob* log(const KUrl& localLocation,
                const KDevelop::VcsRevision& rev,
                unsigned long limit);
    KDevelop::VcsJob* log(const KUrl& localLocation,
                const KDevelop::VcsRevision& rev,
                const KDevelop::VcsRevision& limit);
    KDevelop::VcsJob* annotate(const KUrl& localLocation,
                            const KDevelop::VcsRevision& rev);

    KDevelop::VcsJob* reset(const KUrl& repository, const QStringList &args, const KUrl::List &files);

    DVcsJob* switchBranch(const QString &repository, const QString &branch);
    DVcsJob* branch(const QString &repository, const QString &basebranch = QString(), const QString &branch = QString(),
                    const QStringList &args = QStringList());

    KDevelop::VcsJob* push(const KUrl& localRepositoryLocation,
                          const KDevelop::VcsLocation& localOrRepoLocationDst);
    KDevelop::VcsJob* pull(const KDevelop::VcsLocation& localOrRepoLocationSrc,
                          const KUrl& localRepositoryLocation);

public:
    //parsers for branch:
    QString curBranch(const QString &repository);
    QStringList branches(const QString &repository);

    //graph helpers
    QList<DVcsEvent> getAllCommits(const QString &repo);

protected slots:
    void parseLogOutputBasicVersionControl(DVcsJob *job) const;
    bool parseStatus(DVcsJob *job) const;
    bool parseAnnotations(DVcsJob *job) const;
    void parseDiff(DVcsJob *job) const;

protected:
    //used in log
    void parseLogOutput(const DVcsJob *job, QList<DVcsEvent>& commits) const;

    QString toMercurialRevision(const KDevelop::VcsRevision & vcsrev);
    static bool addDirsConditionally(DVcsJob* job, const KUrl::List & locations, KDevelop::IBasicVersionControl::RecursionMode recursion);
    static KDevelop::VcsStatusInfo::State charToState(const char ch);

    QStringList getLsFiles(const QString &directory, const QStringList &args = QStringList());
};

#endif
