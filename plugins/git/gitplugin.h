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

#ifndef KDEVPLATFORM_PLUGIN_GIT_PLUGIN_H
#define KDEVPLATFORM_PLUGIN_GIT_PLUGIN_H

#include <vcs/interfaces/idistributedversioncontrol.h>
#include <vcs/interfaces/icontentawareversioncontrol.h>
#include <vcs/dvcs/dvcsplugin.h>
#include <QObject>
#include <QProcess>
#include <vcs/vcsstatusinfo.h>
#include <outputview/outputjob.h>
#include <vcs/vcsjob.h>

class KDirWatch;
class QDir;

namespace KDevelop
{
    class VcsJob;
    class VcsRevision;
}

class StandardJob : public KDevelop::VcsJob
{
    Q_OBJECT
    public:
        StandardJob(KDevelop::IPlugin* parent, KJob* job, OutputJobVerbosity verbosity);
        
        virtual QVariant fetchResults() { return QVariant(); }
        virtual void start();
        virtual JobStatus status() const { return m_status; }
        virtual KDevelop::IPlugin* vcsPlugin() const { return m_plugin; }
        
    public slots:
        void result(KJob*);
    
    private:
        KJob* m_job;
        KDevelop::IPlugin* m_plugin;
        JobStatus m_status;
};

/**
 * This is the main class of KDevelop's Git plugin.
 *
 * It implements the DVCS dependent things not implemented in KDevelop::DistributedVersionControlPlugin
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class GitPlugin: public KDevelop::DistributedVersionControlPlugin, public KDevelop::IContentAwareVersionControl
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::IDistributedVersionControl KDevelop::IContentAwareVersionControl)
    friend class GitInitTest;
public:
    GitPlugin(QObject *parent, const QVariantList & args = QVariantList() );
    ~GitPlugin();
   
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
    virtual void setupCommitMessageEditor(const KUrl& localLocation, KTextEdit* editor) const;
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

    virtual KDevelop::VcsJob* log( const KUrl& localLocation, const KDevelop::VcsRevision& rev, unsigned long limit);
    virtual KDevelop::VcsJob* log(const KUrl& localLocation, const KDevelop::VcsRevision& rev, const KDevelop::VcsRevision& limit);
    KDevelop::VcsJob* annotate(const KUrl &localLocation, const KDevelop::VcsRevision &rev);
    KDevelop::VcsJob* revert(const KUrl::List& localLocations, RecursionMode recursion);

    // Begin:  KDevelop::IDistributedVersionControl
    KDevelop::VcsJob* init(const KUrl & directory);

    // Branch management
    KDevelop::VcsJob* tag(const KUrl& repository, const QString& commitMessage, const KDevelop::VcsRevision& rev, const QString& tagName);
    KDevelop::VcsJob* branch(const KUrl& repository, const KDevelop::VcsRevision& rev, const QString& branchName);
    KDevelop::VcsJob* branches(const KUrl& repository);
    KDevelop::VcsJob* currentBranch(const KUrl& repository);
    KDevelop::VcsJob* deleteBranch(const KUrl& repository, const QString& branchName);
    KDevelop::VcsJob* switchBranch(const KUrl& repository, const QString& branchName);
    KDevelop::VcsJob* renameBranch(const KUrl& repository, const QString& oldBranchName, const QString& newBranchName);

    //graph helpers
    QList<DVcsEvent> getAllCommits(const QString &repo);

    //used in log
    void parseLogOutput(const KDevelop::DVcsJob * job,
                        QList<DVcsEvent>& commits) const;

    virtual void additionalMenuEntries(QMenu* menu, const KUrl::List& urls);
    
    KDevelop::DVcsJob* gitStash(const QDir& repository, const QStringList& args, KDevelop::OutputJob::OutputJobVerbosity verbosity);
    
    bool hasStashes(const QDir& repository);
    bool hasModifications(const QDir& repository);
    bool hasModifications(const QDir& repo, const KUrl& file);

    virtual bool hasError() const;
    virtual QString errorDescription() const;
    virtual void registerRepositoryForCurrentBranchChanges(const KUrl& repository);

    KDevelop::CheckInRepositoryJob* isInRepository(KTextEditor::Document* document);
protected:
  
    KUrl repositoryRoot(const KUrl& path);
  
    bool isValidDirectory(const KUrl &dirPath);

    KDevelop::DVcsJob* lsFiles(const QDir &repository,
                     const QStringList &args,
                     KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
    KDevelop::DVcsJob* gitRevList(const QString &directory,
                        const QStringList &args);
    KDevelop::DVcsJob* gitRevParse(const QString &repository,
                         const QStringList &args,
                         KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Silent);

private slots:
    void parseGitBlameOutput(KDevelop::DVcsJob *job);
    void parseGitLogOutput(KDevelop::DVcsJob *job);
    void parseGitDiffOutput(KDevelop::DVcsJob* job);
    void parseGitRepoLocationOutput(KDevelop::DVcsJob* job);
    void parseGitStatusOutput(KDevelop::DVcsJob* job);
    void parseGitStatusOutput_old(KDevelop::DVcsJob* job);
    void parseGitVersionOutput(KDevelop::DVcsJob* job);
    void parseGitBranchOutput(KDevelop::DVcsJob* job);
    void parseGitCurrentBranch(KDevelop::DVcsJob* job);
    
    void ctxPushStash();
    void ctxPopStash();
    void ctxStashManager();

    void fileChanged(const QString& file);
    void delayedBranchChanged();

signals:
    void repositoryBranchChanged(const KUrl& repository);

private:
    void addNotVersionedFiles(const QDir& dir, const KUrl::List& files);
    
    //commit dialog "main" helper
    QStringList getLsFiles(const QDir &directory, const QStringList &args,
        KDevelop::OutputJob::OutputJobVerbosity verbosity);
    KDevelop::DVcsJob* errorsFound(const QString& error, KDevelop::OutputJob::OutputJobVerbosity verbosity);

    void initBranchHash(const QString &repo);

    static KDevelop::VcsStatusInfo::State messageToState(const QString& ch);

    QList<QStringList> branchesShas;
    KUrl::List m_urls;
    
    /** Tells if it's older than 1.7.0 or not */
    bool m_oldVersion;

    bool m_hasError;
    QString m_errorDescription;
    KDirWatch* m_watcher;
    KUrl::List m_branchesChange;
};

QVariant runSynchronously(KDevelop::VcsJob* job);

#endif
