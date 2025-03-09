/*
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>
    SPDX-FileCopyrightText: 2009 Hugo Parente Lima <hugo.pl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_GIT_PLUGIN_H
#define KDEVPLATFORM_PLUGIN_GIT_PLUGIN_H

#include <vcs/interfaces/idistributedversioncontrol.h>
#include <vcs/interfaces/icontentawareversioncontrol.h>
#include <vcs/dvcs/dvcsplugin.h>
#include <vcs/vcsstatusinfo.h>
#include <outputview/outputjob.h>
#include <vcs/vcsjob.h>

#include <QDateTime>
#include <QVarLengthArray>

#include <memory>
#include <vector>

class KDirWatch;
class QDir;
class QTimer;

class CommitToolViewFactory;

namespace KDevelop
{
    class VcsJob;
    class VcsRevision;
}

class StandardJob : public KDevelop::VcsJob
{
    Q_OBJECT
    public:
        explicit StandardJob(KDevelop::IPlugin* parent, KJob* job);

        QVariant fetchResults() override { return QVariant(); }
        void start() override;
        JobStatus status() const override { return m_status; }
        KDevelop::IPlugin* vcsPlugin() const override { return m_plugin; }

    public Q_SLOTS:
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

    enum ExtendedState {
        /* Unchanged in index (no staged changes) */
        GitXX = KDevelop::VcsStatusInfo::ItemUserState, // No changes in worktree

               // Changed in worktree, not staged for commit
        GitXM, // Modified in worktree
        GitXD, // Deleted in worktree
        GitXR, // Renamed in worktree
        GitXC, // Copied in worktree

        /* Changes in index (staged changes) */
        GitMX, // No changes in worktree
                  // Changed in worktree, not staged for commit
        GitMM, // Modified in worktree
        GitMD, // Deleted in worktree

        /* Added to index (new item) */
        GitAX, // No changes in worktree
                  // Changes in worktree, not staged for commit
        GitAM, // Modified in worktree
        GitAD, // Deleted in worktree

        /* Deleted from index */
        GitDX, // No changes in worktree (deleted in wt)
        GitDR, // Renamed in worktree
        GitDC, // Copied in worktree

        /* Renamed in index */
        GitRX, // No changes in worktree
        GitRM, // Modified in worktree
        GitRD, // Deleted in worktree

        /* Copied in index */
        GitCX, // No changes in worktree
        GitCM, // Modified in worktree
        GitCD, // Deleted in worktree

        /* Special states */
        GitUntracked, // ? ? --- untracked files
        GitConflicts, // U, AA, DD --- conflicts
        GitInvalid = -1, // not really a state
    };

    /**
     * Enums with values which are used as function arguments
     * instead of bools for better readability.
     *
     * The enums are named ${function_name}Params.
     */
    enum ApplyParams {
        Index = 0,
        WorkTree = 2,
    };

    explicit GitPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args = QVariantList());
    ~GitPlugin() override;

    void unload() override;

    QString name() const override;

    bool isValidRemoteRepositoryUrl(const QUrl& remoteLocation) override;
    bool isVersionControlled(const QUrl &path) override;

    KDevelop::VcsJob* copy(const QUrl& localLocationSrc, const QUrl& localLocationDstn) override;
    KDevelop::VcsJob* move(const QUrl& localLocationSrc, const QUrl& localLocationDst) override;

    //TODO
    KDevelop::VcsJob* pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const QUrl& localRepositoryLocation) override;
    KDevelop::VcsJob* push(const QUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst) override;
    KDevelop::VcsJob* repositoryLocation(const QUrl& localLocation) override;
    KDevelop::VcsJob* resolve(const QList<QUrl>& localLocations, RecursionMode recursion) override;
    KDevelop::VcsJob* update(const QList<QUrl>& localLocations, const KDevelop::VcsRevision& rev, RecursionMode recursion) override;
    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const override;
    void setupCommitMessageEditor(const QUrl& localLocation, KTextEdit* editor) const override;
    //End of

    KDevelop::VcsJob* add(const QList<QUrl>& localLocations,
                          KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive) override;
    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & localOrRepoLocationSrc,
                            const QUrl& localRepositoryRoot, KDevelop::IBasicVersionControl::RecursionMode) override;

    KDevelop::VcsJob* remove(const QList<QUrl>& files) override;
    KDevelop::VcsJob* status(const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive) override;
    KDevelop::VcsJob* commit(const QString& message,
                             const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive) override;

    /**
     * Commits staged changes to the repo located at repoUrl.
     *
     * @param message the commit message
     * @param repoUrl the url pointing to the repo directory (or a file in the repo)
     */
    KDevelop::VcsJob* commitStaged(const QString& message, const QUrl& repoUrl);

    KDevelop::VcsJob* diff(const QUrl& fileOrDirectory, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision,
                                   RecursionMode recursion) override;
    /**
     * Shows a diff of changes between srcRevision and dstRevision.
     *
     * @param repoPath a path pointing somewhere inside the repo
     * @param srcRevision the source revision
     * @param dstRevision the destination revision
     *
     * @note: This differs from the @ref:diff method in @ref:IBasicVersionControl in that it does not require
     * a list of files but automatically shows all changed files
     */
    KDevelop::VcsJob* diff(const QUrl& repoPath, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision);

    KDevelop::VcsJob* log( const QUrl& localLocation, const KDevelop::VcsRevision& rev, unsigned long limit) override;
    KDevelop::VcsJob* log(const QUrl& localLocation, const KDevelop::VcsRevision& rev, const KDevelop::VcsRevision& limit) override;
    KDevelop::VcsJob* annotate(const QUrl &localLocation, const KDevelop::VcsRevision &rev) override;
    KDevelop::VcsJob* revert(const QList<QUrl>& localLocations, RecursionMode recursion) override;

    /**
     * Resets all changes in the specified files which were "staged for commit".
     *
     * @param localLocations the local files/dirs changes to which should be reset
     * @param recursion defines whether changes should be reset recursively in all files
     * in a directory, if localLocations contain a directory
     */
    KDevelop::VcsJob* reset(const QList<QUrl>& localLocations, RecursionMode recursion);

    /**
     * Applies the patch given by a diff to the repo
     *
     * @param diff the patch
     * @param applyTo where to apply the patch (index or worktree)
     */
    KDevelop::VcsJob* apply(const KDevelop::VcsDiff& diff, ApplyParams applyTo = WorkTree);

    // Begin:  KDevelop::IDistributedVersionControl
    KDevelop::VcsJob* init(const QUrl & directory) override;

    // Branch management
    KDevelop::VcsJob* tag(const QUrl& repository, const QString& commitMessage, const KDevelop::VcsRevision& rev, const QString& tagName) override;
    KDevelop::VcsJob* branch(const QUrl& repository, const KDevelop::VcsRevision& rev, const QString& branchName) override;
    KDevelop::VcsJob* branches(const QUrl& repository) override;
    KDevelop::VcsJob* currentBranch(const QUrl& repository) override;
    KDevelop::VcsJob* deleteBranch(const QUrl& repository, const QString& branchName) override;
    KDevelop::VcsJob* switchBranch(const QUrl& repository, const QString& branchName) override;
    KDevelop::VcsJob* renameBranch(const QUrl& repository, const QString& oldBranchName, const QString& newBranchName) override;
    KDevelop::VcsJob* mergeBranch(const QUrl& repository, const QString& branchName) override;
    KDevelop::VcsJob* rebase(const QUrl& repository, const QString& branchName);

    //graph helpers
    QVector<KDevelop::DVcsEvent> allCommits(const QString& repo) override;

    //used in log
    void parseLogOutput(const KDevelop::DVcsJob* job,
                        QVector<KDevelop::DVcsEvent>& commits) const override;

    void additionalMenuEntries(QMenu* menu, const QList<QUrl>& urls) override;

    // Stash Management

    /**
     * Structure to hold information about an item on the stash stack
     */
    struct StashItem {
        int stackDepth = -1;        /* Position on the stack */
        QString shortRef;           /* The reflog selector (e.g. stash@{0}) */
        QString parentSHA;          /* The short SHA of the commit on which the stash was made */
        QString parentDescription;  /* A short description of the commit on which the stash was made */
        QString branch;             /* The branch on which the stash was made */
        QString message;            /* The message with which the stash was made */
        QDateTime creationTime;     /* The date-time the stash item was committed */
    };

    /**
     * Returns a job to run `git stash` in the repository @p repository with
     * additional arguments @p args.
     *
     * The @p verbosity parameter will determine whether the job output will
     * be shown in the VCS Output ToolView.
     *
     * For example, a job to silently apply the top-most stashed item to the current
     * tree would be created as follows:
     *
     *      gitStash(repoDir, {QStringLiteral("apply")}, KDevelop::OutputJob::Silent)
     *
     */
    KDevelop::VcsJob* gitStash(const QDir& repository, const QStringList& args, KDevelop::OutputJob::OutputJobVerbosity verbosity);

    /**
     * The result (job->fetchResults()) will be a @ref QList of @ref StashItem s
     *
     * @p repository is the repository to work on
     * @p verbosity  determines whether the job output will be shown in the VCS Output ToolView
     */
    KDevelop::VcsJob* stashList(const QDir& repository, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Silent);

    bool hasStashes(const QDir& repository);
    bool hasModifications(const QDir& repository);
    bool hasModifications(const QDir& repo, const QUrl& file);

    void registerRepositoryForCurrentBranchChanges(const QUrl& repository, const QObject* listener) override;
    void unregisterRepositoryForCurrentBranchChanges(const QUrl& repository, const QObject* listener) override;

    KDevelop::CheckInRepositoryJob* isInRepository(KTextEditor::Document* document) override;

    KDevelop::DVcsJob* setConfigOption(const QUrl& repository, const QString& key, const QString& value, bool global = false);
    QString readConfigOption(const QUrl& repository, const QString& key);

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

    bool isValidDirectory(const QUrl &dirPath) override;

    KDevelop::DVcsJob* lsFiles(const QDir &repository,
                     const QStringList &args,
                     KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
    KDevelop::DVcsJob* gitRevList(const QString &directory,
                        const QStringList &args);
    KDevelop::DVcsJob* gitRevParse(const QString &repository,
                         const QStringList &args,
                         KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Silent);

private Q_SLOTS:
    void parseGitBlameOutput(KDevelop::DVcsJob *job);
    void parseGitLogOutput(KDevelop::DVcsJob *job);
    void parseGitDiffOutput(KDevelop::DVcsJob* job);
    void parseGitRepoLocationOutput(KDevelop::DVcsJob* job);
    void parseGitStatusOutput(KDevelop::DVcsJob* job);
    void parseGitStatusOutput_old(KDevelop::DVcsJob* job);
    void parseGitVersionOutput(KDevelop::DVcsJob* job);
    void parseGitBranchOutput(KDevelop::DVcsJob* job);
    void parseGitCurrentBranch(KDevelop::DVcsJob* job);
    void parseGitStashList(KDevelop::VcsJob* job);

    void ctxRebase();
    void ctxPushStash();
    void ctxPopStash();
    void ctxStashManager();

    void fileChanged(const QString& file);
    void delayedBranchChanged();

Q_SIGNALS:
    void repositoryBranchChanged(const QUrl& repository);

private:
    class WatchedFile
    {
    public:
        explicit WatchedFile(const QString& filePath);

        [[nodiscard]] const QString& filePath() const;

        /**
         * @return whether a given object is the timer to invoke delayedBranchChanged() asynchronously owned by @c this
         *
         * @param object a non-null object
         */
        [[nodiscard]] bool isOwnTimer(const QObject* object) const;

        /**
         * Invoke @p plugin->delayedBranchChanged() in a little while.
         *
         * @param plugin a non-null plugin instance. The plugin pointer must be the same
         *        each time because the plugin's delayedBranchChanged() slot is connected
         *        to the timer's @c timeout() signal only during the first call to this function.
         */
        void scheduleDelayedBranchChanged(const GitPlugin* plugin);

    private:
        /**
         * The path to the watched file.
         *
         * The file path is logically constant but is not declared as
         * @c const to allow std::vector to move WatchedFile elements.
         */
        QString m_filePath;
        /**
         * The timer to invoke delayedBranchChanged() asynchronously.
         *
         * The default deleter is safe because the timer's @c timeout()
         * signal does not trigger destruction of a WatchedFile.
         */
        std::unique_ptr<QTimer> m_timer;
    };

    /**
     * This class combines two independent entities: class WatchedFile
     * and a container of listeners for changes to the watched file.
     */
    class WatchedFileAndListeners : public WatchedFile
    {
    public:
        using WatchedFile::WatchedFile;

        /**
         * Listeners that have registered filePath() for current branch changes.
         *
         * Storing the listeners mitigates the consequences of unregistering a listener for a repository without
         * having previously registered it. This might happen if a project is opened, then a not-yet-listener
         * object is destroyed and unregisters itself, never having received the @c projectOpened() signal.
         *
         * The number of listeners normally should not exceed 3, hence the value of the template argument @c Prealloc.
         */
        QVarLengthArray<const QObject*, 3> listeners;
    };

    [[nodiscard]] std::vector<WatchedFileAndListeners>::iterator findWatchedFile(const QString& filePath);

    bool ensureValidGitIdentity(const QDir& dir);
    void addNotVersionedFiles(const QDir& dir, const QList<QUrl>& files);

    //commit dialog "main" helper
    QStringList getLsFiles(const QDir &directory, const QStringList &args,
        KDevelop::OutputJob::OutputJobVerbosity verbosity);

    void initBranchHash(const QString &repo);

    /**
     * Parses a git status --porcelain line
     *
     * @param statusLine a line as returned by `git status --porcelain`
     * @returns the appropriate extended status
     */
    static ExtendedState parseGitState(QStringView statusLine);

    /**
     * Maps an extended state to a basic state
     *
     * @param state the extended state as provided by git (i.e. describing the combined status in the index & worktree)
     */
    static KDevelop::VcsStatusInfo::State extendedStateToBasic(const ExtendedState state);

    QList<QStringList> branchesShas;
    QList<QUrl> m_urls;

    /** Tells if it's older than 1.7.0 or not */
    bool m_oldVersion = false;

    KDirWatch* m_watcher;
    std::vector<WatchedFileAndListeners> m_watchedFiles;
    bool m_usePrefix = true;

    /** A factory for constructing the tool view for preparing commits */
    CommitToolViewFactory* m_commitToolViewFactory;
};

Q_DECLARE_METATYPE(GitPlugin::StashItem)

QVariant runSynchronously(KDevelop::VcsJob* job);

#endif
