/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   Copyright 2009 Hugo Parente Lima <hugo.pl@gmail.com>                  *
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

#ifndef GIT_PLUGIN_H
#define GIT_PLUGIN_H

#include <vcs/interfaces/idistributedversioncontrol.h>
#include <vcs/dvcs/dvcsplugin.h>
#include <QObject>
#include <vcs/vcsstatusinfo.h>
#include <outputview/outputjob.h>
#include <vcs/vcsjob.h>


namespace KDevelop
{
    class VcsJob;
    class VcsRevision;
}

class StandardCopyJob : public KDevelop::VcsJob
{
    Q_OBJECT
    public:
        StandardCopyJob(KDevelop::IPlugin* parent, const KUrl& source,
                        const KUrl& dest, OutputJobVerbosity verbosity);
        
        virtual QVariant fetchResults() { return QVariant(); }
        virtual void start();
        virtual JobStatus status() const { return m_status; }
        virtual KDevelop::IPlugin* vcsPlugin() const { return m_plugin; }
        
    public slots:
        void result(KJob*);
    
    private:
        const KUrl m_source, m_dest;
        KDevelop::IPlugin* m_plugin;
        JobStatus m_status;
};

/**
 * This is the main class of KDevelop's Git plugin.
 *
 * It implements the DVCS dependent things not implemented in KDevelop::DistributedVersionControlPlugin
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class GitPlugin: public KDevelop::DistributedVersionControlPlugin
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::IDistributedVersionControl)
    friend class GitInitTest;
public:
    GitPlugin(QObject *parent, const QVariantList & args = QVariantList() );
    ~GitPlugin();
   
    virtual void unload();

    QString name() const;

    bool isVersionControlled(const KUrl &path);
    
    virtual KDevelop::VcsJob* copy(const KUrl& localLocationSrc, const KUrl& localLocationDstn);
    virtual KDevelop::VcsJob* move(const KUrl& localLocationSrc, const KUrl& localLocationDst);
    
    //TODO
    virtual KDevelop::VcsJob* pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const KUrl& localRepositoryLocation);
    virtual KDevelop::VcsJob* push(const KUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst);
    virtual KDevelop::VcsJob* repositoryLocation(const KUrl& localLocation);
    virtual KDevelop::VcsJob* resolve(const KUrl::List& localLocations, RecursionMode recursion);
    virtual KDevelop::VcsJob* update(const KUrl::List& localLocations, const KDevelop::VcsRevision& rev, RecursionMode recursion);
    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const;
    //End of

    KDevelop::VcsJob* add(const KUrl::List& localLocations,
                          KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);
    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & localOrRepoLocationSrc,
                            const KUrl& localRepositoryRoot, KDevelop::IBasicVersionControl::RecursionMode);

    KDevelop::VcsJob* remove(const KUrl::List& files);
    KDevelop::VcsJob* status(const KUrl::List& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);
    KDevelop::VcsJob* commit(const QString& message,
                             const KUrl::List& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);

    virtual KDevelop::VcsJob* diff(const KUrl& fileOrDirectory, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision,
                                   KDevelop::VcsDiff::Type, RecursionMode recursion);
    using KDevelop::DistributedVersionControlPlugin::log;

    KDevelop::VcsJob* log(const KUrl& localLocation,
                          const KDevelop::VcsRevision& rev,
                          unsigned long limit);
    KDevelop::VcsJob* annotate(const KUrl &localLocation, const KDevelop::VcsRevision &rev);
    KDevelop::VcsJob* revert(const KUrl::List& localLocations, RecursionMode recursion);

    // Begin:  KDevelop::IDistributedVersionControl
    KDevelop::VcsJob* init(const KUrl & directory);
    KDevelop::VcsJob* reset(const KUrl& repository,
                            const QStringList &args,
                            const KUrl::List& files);
    // End:  KDevelop::IDistributedVersionControl

    DVcsJob* var(const QString &directory);

    // Branch management

    DVcsJob* switchBranch(const QString &repository,
                          const QString &branch);
    DVcsJob* branch(const QString &repository,
                    const QString &basebranch = QString(),
                    const QString &branch = QString(),
                    const QStringList &args = QStringList());

    QString curBranch(const QString &repository);
    QStringList branches(const QString &repository);

    //commit dialog helpers, send to main helper the arg for git-ls-files:
    QList<QVariant> getModifiedFiles(const QString &directory, KDevelop::OutputJob::OutputJobVerbosity verbosity);
    QList<QVariant> getCachedFiles(const QString &directory, KDevelop::OutputJob::OutputJobVerbosity verbosity);
    QList<QVariant> getOtherFiles(const QString &directory, KDevelop::OutputJob::OutputJobVerbosity verbosity);

    //graph helpers
    QList<DVcsEvent> getAllCommits(const QString &repo);

    //used in log
    void parseLogOutput(const DVcsJob * job,
                        QList<DVcsEvent>& commits) const;

protected:
  
    KUrl repositoryRoot(const KUrl& path);
  
    bool isValidDirectory(const KUrl &dirPath);

    DVcsJob* lsFiles(const QString &repository,
                     const QStringList &args,
                     KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
    DVcsJob* gitRevList(const QString &repository,
                        const QStringList &args);
    DVcsJob* gitRevParse(const QString &repository,
                         const QStringList &args,
                         KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Silent);

protected slots:
    void parseGitBlameOutput(DVcsJob *job);
    void parseGitLogOutput(DVcsJob *job);
    void parseGitDiffOutput(DVcsJob* job);
    void parseGitRepoLocationOutput(DVcsJob* job);

private:
    //commit dialog "main" helper
    QStringList getLsFiles(const QString &directory, const QStringList &args,
        KDevelop::OutputJob::OutputJobVerbosity verbosity);
	DVcsJob* errorsFound(const QString& error, KDevelop::OutputJob::OutputJobVerbosity verbosity);

    void initBranchHash(const QString &repo);

    static KDevelop::VcsStatusInfo::State charToState(char ch);

    QList<QStringList> branchesShas;
    // Set when isValidDirectory is called.
    KUrl m_lastRepoRoot;
};

#endif
