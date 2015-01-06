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

    bool isVersionControlled(const QUrl &path);
    
    virtual KDevelop::VcsJob* copy(const QUrl& localLocationSrc, const QUrl& localLocationDstn);
    virtual KDevelop::VcsJob* move(const QUrl& localLocationSrc, const QUrl& localLocationDst);
    
    //TODO
    virtual KDevelop::VcsJob* pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const QUrl& localRepositoryLocation);
    virtual KDevelop::VcsJob* push(const QUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst);
    virtual KDevelop::VcsJob* repositoryLocation(const QUrl& localLocation);
    virtual KDevelop::VcsJob* resolve(const QList<QUrl>& localLocations, RecursionMode recursion);
    virtual KDevelop::VcsJob* update(const QList<QUrl>& localLocations, const KDevelop::VcsRevision& rev, RecursionMode recursion);
    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const;
    virtual void setupCommitMessageEditor(const QUrl& localLocation, KTextEdit* editor) const;
    //End of

    KDevelop::VcsJob* add(const QList<QUrl>& localLocations,
                          KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);
    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & localOrRepoLocationSrc,
                            const QUrl& localRepositoryRoot, KDevelop::IBasicVersionControl::RecursionMode);

    KDevelop::VcsJob* remove(const QList<QUrl>& files);
    KDevelop::VcsJob* status(const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);
    KDevelop::VcsJob* commit(const QString& message,
                             const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);

    virtual KDevelop::VcsJob* diff(const QUrl& fileOrDirectory, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision,
                                   KDevelop::VcsDiff::Type, RecursionMode recursion);

    virtual KDevelop::VcsJob* log( const QUrl& localLocation, const KDevelop::VcsRevision& rev, unsigned long limit);
    virtual KDevelop::VcsJob* log(const QUrl& localLocation, const KDevelop::VcsRevision& rev, const KDevelop::VcsRevision& limit);
    KDevelop::VcsJob* annotate(const QUrl &localLocation, const KDevelop::VcsRevision &rev);
    KDevelop::VcsJob* revert(const QList<QUrl>& localLocations, RecursionMode recursion);

    // Begin:  KDevelop::IDistributedVersionControl
    KDevelop::VcsJob* init(const QUrl & directory);

    // Branch management
    KDevelop::VcsJob* tag(const QUrl& repository, const QString& commitMessage, const KDevelop::VcsRevision& rev, const QString& tagName);
    KDevelop::VcsJob* branch(const QUrl& repository, const KDevelop::VcsRevision& rev, const QString& branchName);
    KDevelop::VcsJob* branches(const QUrl& repository);
    KDevelop::VcsJob* currentBranch(const QUrl& repository);
    KDevelop::VcsJob* deleteBranch(const QUrl& repository, const QString& branchName);
    KDevelop::VcsJob* switchBranch(const QUrl& repository, const QString& branchName);
    KDevelop::VcsJob* renameBranch(const QUrl& repository, const QString& oldBranchName, const QString& newBranchName);

    //graph helpers
    QList<DVcsEvent> getAllCommits(const QString &repo);

    //used in log
    void parseLogOutput(const KDevelop::DVcsJob * job,
                        QList<DVcsEvent>& commits) const;

    virtual void additionalMenuEntries(QMenu* menu, const QList<QUrl>& urls);
    
    KDevelop::DVcsJob* gitStash(const QDir& repository, const QStringList& args, KDevelop::OutputJob::OutputJobVerbosity verbosity);
    
    bool hasStashes(const QDir& repository);
    bool hasModifications(const QDir& repository);
    bool hasModifications(const QDir& repo, const QUrl& file);

    virtual bool hasError() const;
    virtual QString errorDescription() const;
    virtual void registerRepositoryForCurrentBranchChanges(const QUrl& repository);

    KDevelop::CheckInRepositoryJob* isInRepository(KTextEditor::Document* document);

    // this indicates whether the diff() function will generate a diff (patch) which
    // includes the working copy directory name or not (in which case git diff is called
    // with --no-prefix).
    bool usePrefix() const
    {
        return m_usePrefix;
    }

    void setUsePrefix(bool p)
    {
        m_usePrefix = p;
    }
protected:
  
    QUrl repositoryRoot(const QUrl& path);
  
    bool isValidDirectory(const QUrl &dirPath);

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
    void repositoryBranchChanged(const QUrl& repository);

private:
    void addNotVersionedFiles(const QDir& dir, const QList<QUrl>& files);
    
    //commit dialog "main" helper
    QStringList getLsFiles(const QDir &directory, const QStringList &args,
        KDevelop::OutputJob::OutputJobVerbosity verbosity);
    KDevelop::DVcsJob* errorsFound(const QString& error, KDevelop::OutputJob::OutputJobVerbosity verbosity);

    void initBranchHash(const QString &repo);

    static KDevelop::VcsStatusInfo::State messageToState(const QString& ch);

    QList<QStringList> branchesShas;
    QList<QUrl> m_urls;
    
    /** Tells if it's older than 1.7.0 or not */
    bool m_oldVersion;

    bool m_hasError;
    QString m_errorDescription;
    KDirWatch* m_watcher;
    QList<QUrl> m_branchesChange;
    bool m_usePrefix;
};

QVariant runSynchronously(KDevelop::VcsJob* job);

#endif
