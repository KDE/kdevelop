/*
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>
    SPDX-FileCopyrightText: 2009 Hugo Parente Lima <hugo.pl@gmail.com>
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gitplugin.h"

#include "repostatusmodel.h"
#include "committoolview.h"

#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QMenu>
#include <QTimer>
#include <QRegExp>
#include <QRegularExpression>
#include <QPointer>
#include <QTemporaryFile>
#include <QVersionNumber>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>

#include <util/path.h>
#include <util/stringviewhelpers.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>
#include <vcs/vcslocation.h>
#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include <vcs/widgets/standardvcslocationwidget.h>
#include "gitclonejob.h"
#include "rebasedialog.h"
#include "stashmanagerdialog.h"

#include <KDirWatch>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEdit>
#include <KTextEditor/Document>

#include "gitjob.h"
#include "gitmessagehighlighter.h"
#include "gitplugincheckinrepositoryjob.h"
#include "gitnameemaildialog.h"
#include "debug.h"

#include <algorithm>
#include <array>
#include <utility>

using namespace KDevelop;

QVariant runSynchronously(KDevelop::VcsJob* job)
{
    QVariant ret;
    if(job->exec() && job->status()==KDevelop::VcsJob::JobSucceeded) {
        ret = job->fetchResults();
    }
    delete job;
    return ret;
}

namespace
{

QDir dotGitDirectory(const QUrl& dirPath, bool silent = false)
{
    const QFileInfo finfo(dirPath.toLocalFile());
    QDir dir = finfo.isDir() ? QDir(finfo.filePath()): finfo.absoluteDir();

    const QString gitDir = QStringLiteral(".git");
    while (!dir.exists(gitDir) && dir.cdUp()) {} // cdUp, until there is a sub-directory called .git

    if (!silent && dir.isRoot()) {
        qCWarning(PLUGIN_GIT) << "couldn't find the git root for" << dirPath;
    }

    return dir;
}

[[nodiscard]] QString gitHeadFile(const QUrl& dirPath)
{
    const auto dir = dotGitDirectory(dirPath);
    return dir.absoluteFilePath(QStringLiteral(".git/HEAD"));
}

/**
 * Whenever a directory is provided, change it for all the files in it but not inner directories,
 * that way we make sure we won't get into recursion,
 */
static QList<QUrl> preventRecursion(const QList<QUrl>& urls)
{
    QList<QUrl> ret;
    for (const QUrl& url : urls) {
        QDir d(url.toLocalFile());
        if(d.exists()) {
            const QStringList entries = d.entryList(QDir::Files | QDir::NoDotAndDotDot);
            ret.reserve(ret.size() + entries.size());
            for (const QString& entry : entries) {
                QUrl entryUrl = QUrl::fromLocalFile(d.absoluteFilePath(entry));
                ret += entryUrl;
            }
        } else
            ret += url;
    }
    return ret;
}

QString toRevisionName(const KDevelop::VcsRevision& rev, const QString& currentRevision=QString())
{
    switch(rev.revisionType()) {
        case VcsRevision::Special:
            switch(rev.revisionValue().value<VcsRevision::RevisionSpecialType>()) {
                case VcsRevision::Head:
                    return QStringLiteral("^HEAD");
                case VcsRevision::Base:
                    return QString();
                case VcsRevision::Working:
                    return QString();
                case VcsRevision::Previous:
                    Q_ASSERT(!currentRevision.isEmpty());
                    return currentRevision + QLatin1String("^1");
                case VcsRevision::Start:
                    return QString();
                case VcsRevision::UserSpecialType: //Not used
                    Q_ASSERT(false && "i don't know how to do that");
            }
            break;
        case VcsRevision::GlobalNumber:
            return rev.revisionValue().toString();
        case VcsRevision::Date:
        case VcsRevision::FileNumber:
        case VcsRevision::Invalid:
        case VcsRevision::UserType:
            Q_ASSERT(false);
    }
    return QString();
}

QString revisionInterval(const KDevelop::VcsRevision& rev, const KDevelop::VcsRevision& limit)
{
    QString ret;
    if(rev.revisionType()==VcsRevision::Special &&
                rev.revisionValue().value<VcsRevision::RevisionSpecialType>()==VcsRevision::Start) //if we want it to the beginning just put the revisionInterval
        ret = toRevisionName(limit, QString());
    else {
        QString dst = toRevisionName(limit);
        if(dst.isEmpty())
            ret = dst;
        else {
            QString src = toRevisionName(rev, dst);
            if(src.isEmpty())
                ret = src;
            else
                ret = src + QLatin1String("..") + dst;
        }
    }
    return ret;
}

QDir urlDir(const QUrl& url)
{
    QFileInfo f(url.toLocalFile());
    if(f.isDir())
        return QDir(url.toLocalFile());
    else
        return f.absoluteDir();
}
QDir urlDir(const QList<QUrl>& urls) { return urlDir(urls.first()); } //TODO: could be improved

}

GitPlugin::GitPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : DistributedVersionControlPlugin(QStringLiteral("kdevgit"), parent, metaData)
    , m_repoStatusModel(new RepoStatusModel(this))
    , m_commitToolViewFactory(new CommitToolViewFactory(m_repoStatusModel))
{
    if (QStandardPaths::findExecutable(QStringLiteral("git")).isEmpty()) {
        setErrorDescription(i18n("Unable to find git executable. Is it installed on the system?"));
        return;
    }

    // FIXME: Is this needed (I don't quite understand the comment
    // in vcsstatusinfo.h which says we need to do this if we want to
    // use VcsStatusInfo in queued signals/slots)
    qRegisterMetaType<VcsStatusInfo>();

    ICore::self()->uiController()->addToolView(i18n("Git Commit"), m_commitToolViewFactory);

    setObjectName(QStringLiteral("Git"));

    auto* versionJob = new GitJob(QDir::tempPath(), this, KDevelop::OutputJob::Silent);
    *versionJob << "git" << "--version";
    connect(versionJob, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitVersionOutput);
    ICore::self()->runController()->registerJob(versionJob);

    m_watcher = new KDirWatch(this);
    connect(m_watcher, &KDirWatch::dirty, this, &GitPlugin::fileChanged);
    connect(m_watcher, &KDirWatch::created, this, &GitPlugin::fileChanged);
}

GitPlugin::~GitPlugin()
{}

void GitPlugin::unload()
{
    core()->uiController()->removeToolView(m_commitToolViewFactory);
}

bool emptyOutput(DVcsJob* job)
{
    QScopedPointer<DVcsJob> _job(job);
    if(job->exec() && job->status()==VcsJob::JobSucceeded)
        return job->rawOutput().trimmed().isEmpty();

    return false;
}

bool GitPlugin::hasStashes(const QDir& repository)
{
    if (auto *job = qobject_cast<DVcsJob*>(gitStash(repository, QStringList(QStringLiteral("list")), KDevelop::OutputJob::Silent))) {
        return !emptyOutput(job);
    }
    Q_ASSERT(false); // gitStash should always return a DVcsJob !
    return false;
}

bool GitPlugin::hasModifications(const QDir& d)
{
    return !emptyOutput(lsFiles(d, QStringList(QStringLiteral("-m")), OutputJob::Silent));
}

bool GitPlugin::hasModifications(const QDir& repo, const QUrl& file)
{
    return !emptyOutput(lsFiles(repo, QStringList{QStringLiteral("-m"), file.path()}, OutputJob::Silent));
}

void GitPlugin::additionalMenuEntries(QMenu* menu, const QList<QUrl>& urls)
{
    m_urls = urls;

    QDir dir=urlDir(urls);
    bool hasSt = hasStashes(dir);

    menu->addAction(i18nc("@action:inmenu", "Rebase"), this, SLOT(ctxRebase()));
    menu->addSeparator()->setText(i18nc("@title:menu", "Git Stashes"));
    menu->addAction(i18nc("@action:inmenu", "Stash Manager"), this, SLOT(ctxStashManager()))->setEnabled(hasSt);
    menu->addAction(QIcon::fromTheme(QStringLiteral("vcs-stash")), i18nc("@action:inmenu", "Push Stash"), this, SLOT(ctxPushStash()));
    menu->addAction(QIcon::fromTheme(QStringLiteral("vcs-stash-pop")), i18nc("@action:inmenu", "Pop Stash"), this, SLOT(ctxPopStash()))->setEnabled(hasSt);
}

void GitPlugin::ctxRebase()
{
    auto* dialog = new RebaseDialog(this, m_urls.first(), nullptr);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->open();
}

void GitPlugin::ctxPushStash()
{
    VcsJob* job = gitStash(urlDir(m_urls), QStringList(), KDevelop::OutputJob::Verbose);
    ICore::self()->runController()->registerJob(job);
}

void GitPlugin::ctxPopStash()
{
    VcsJob* job = gitStash(urlDir(m_urls), QStringList(QStringLiteral("pop")), KDevelop::OutputJob::Verbose);
    ICore::self()->runController()->registerJob(job);
}

void GitPlugin::ctxStashManager()
{
    QPointer<StashManagerDialog> d = new StashManagerDialog(urlDir(m_urls), this, nullptr);
    d->exec();

    delete d;
}

QString GitPlugin::name() const
{
    return QStringLiteral("Git");
}

QUrl GitPlugin::repositoryRoot(const QUrl& path)
{
    return QUrl::fromLocalFile(dotGitDirectory(path).absolutePath());
}

bool GitPlugin::isValidDirectory(const QUrl & dirPath)
{
    QDir dir = dotGitDirectory(dirPath, true);
    QFile dotGitPotentialFile(dir.filePath(QStringLiteral(".git")));
    // if .git is a file, we may be in a git worktree
    QFileInfo dotGitPotentialFileInfo(dotGitPotentialFile);
    if (!dotGitPotentialFileInfo.isDir() && dotGitPotentialFile.exists()) {
        QString gitWorktreeFileContent;
        if (dotGitPotentialFile.open(QFile::ReadOnly)) {
            // the content should be gitdir: /path/to/the/.git/worktree
            gitWorktreeFileContent = QString::fromUtf8(dotGitPotentialFile.readAll());
            dotGitPotentialFile.close();
        } else {
            return false;
        }
        const auto items = gitWorktreeFileContent.split(QLatin1Char(' '));
        if (items.size() == 2 && items.at(0) == QLatin1String("gitdir:")) {
            qCDebug(PLUGIN_GIT) << "we are in a git worktree" << items.at(1);
            return true;
        }
    }
    return dir.exists(QStringLiteral(".git/HEAD"));
}

bool GitPlugin::isValidRemoteRepositoryUrl(const QUrl& remoteLocation)
{
    if (remoteLocation.isLocalFile()) {
        QFileInfo fileInfo(remoteLocation.toLocalFile());
        if (fileInfo.isDir()) {
            QDir dir(fileInfo.filePath());
            if (dir.exists(QStringLiteral(".git/HEAD"))) {
                return true;
            }
            // TODO: check also for bare repo
        }
    } else {
        const QString scheme = remoteLocation.scheme();
        if (scheme == QLatin1String("git") || scheme == QLatin1String("git+ssh")) {
            return true;
        }
        // heuristic check, anything better we can do here without talking to server?
        if ((scheme == QLatin1String("http") ||
             scheme == QLatin1String("https")) &&
            remoteLocation.path().endsWith(QLatin1String(".git"))) {
            return true;
        }
    }
    return false;
}

bool GitPlugin::isVersionControlled(const QUrl &path)
{
    QFileInfo fsObject(path.toLocalFile());
    if (!fsObject.exists()) {
        return false;
    }
    if (fsObject.isDir()) {
        return isValidDirectory(path);
    }

    QString filename = fsObject.fileName();

    QStringList otherFiles = getLsFiles(fsObject.dir(), QStringList(QStringLiteral("--")) << filename, KDevelop::OutputJob::Silent);
    return !otherFiles.empty();
}

VcsJob* GitPlugin::init(const QUrl &directory)
{
    auto* job = new GitJob(urlDir(directory), this);
    job->setType(VcsJob::Import);
    *job << "git" << "init";
    return job;
}

VcsJob* GitPlugin::createWorkingCopy(const KDevelop::VcsLocation & source, const QUrl& dest, KDevelop::IBasicVersionControl::RecursionMode)
{
    DVcsJob* job = new GitCloneJob(urlDir(dest), this);
    job->setType(VcsJob::Import);
    *job << "git" << "clone" << "--progress" << "--" << source.localUrl().url() << dest;
    return job;
}

VcsJob* GitPlugin::add(const QList<QUrl>& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    if (localLocations.empty())
        return makeVcsErrorJob(i18n("Did not specify the list of files"), this, OutputJob::Verbose);

    DVcsJob* job = new GitJob(dotGitDirectory(localLocations.front()), this);
    job->setType(VcsJob::Add);
    *job << "git" << "add" << "--" << (recursion == IBasicVersionControl::Recursive ? localLocations : preventRecursion(localLocations));
    return job;
}

KDevelop::VcsJob* GitPlugin::status(const QList<QUrl>& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    if (localLocations.empty())
        return makeVcsErrorJob(i18n("Did not specify the list of files"), this, OutputJob::Verbose);

    DVcsJob* job = new GitJob(urlDir(localLocations), this, OutputJob::Silent);
    job->setType(VcsJob::Status);

    if(m_oldVersion) {
        *job << "git" << "ls-files" << "-t" << "-m" << "-c" << "-o" << "-d" << "-k" << "--directory";
        connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitStatusOutput_old);
    } else {
        *job << "git" << "status" << "--porcelain";
        job->setIgnoreError(true);
        connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitStatusOutput);
    }
    *job << "--" << (recursion == IBasicVersionControl::Recursive ? localLocations : preventRecursion(localLocations));

    return job;
}

VcsJob* GitPlugin::diff(const QUrl& fileOrDirectory, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision,
                        IBasicVersionControl::RecursionMode recursion)
{
    DVcsJob* job = static_cast<DVcsJob*>(diff(fileOrDirectory, srcRevision, dstRevision));
    *job << "--";
    if (recursion == IBasicVersionControl::Recursive) {
        *job << fileOrDirectory;
    } else {
        *job << preventRecursion(QList<QUrl>() << fileOrDirectory);
    }
    return job;
}

KDevelop::VcsJob * GitPlugin::diff(const QUrl& repoPath, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision)
{
    DVcsJob* job = new GitJob(dotGitDirectory(repoPath), this, KDevelop::OutputJob::Silent);
    job->setType(VcsJob::Diff);
    *job << "git" << "diff" << "--no-color" << "--no-ext-diff";
    if (!usePrefix()) {
        // KDE's ReviewBoard now requires p1 patchfiles, so `git diff --no-prefix` to generate p0 patches
        // has become optional.
        *job << "--no-prefix";
    }
    if (dstRevision.revisionType() == VcsRevision::Special &&
         dstRevision.specialType() == VcsRevision::Working) {
        if (srcRevision.revisionType() == VcsRevision::Special &&
             srcRevision.specialType() == VcsRevision::Base) {
            *job << "HEAD";
        } else {
            *job << "--cached" << srcRevision.revisionValue().toString();
        }
    } else {
        QString revstr = revisionInterval(srcRevision, dstRevision);
        if(!revstr.isEmpty())
            *job << revstr;
    }
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitDiffOutput);
    return job;
}


KDevelop::VcsJob * GitPlugin::reset ( const QList<QUrl>& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    if(localLocations.isEmpty() )
        return makeVcsErrorJob(i18n("Could not reset changes (empty list of paths)"), this, OutputJob::Verbose);

    DVcsJob* job = new GitJob(dotGitDirectory(localLocations.front()), this);
    job->setType(VcsJob::Reset);
    *job << "git" << "reset" << "--";
    *job << (recursion == IBasicVersionControl::Recursive ? localLocations : preventRecursion(localLocations));
    return job;
}

KDevelop::VcsJob * GitPlugin::apply(const KDevelop::VcsDiff& diff, const ApplyParams applyTo)
{
    DVcsJob* job = new GitJob(dotGitDirectory(diff.baseDiff()), this);
    job->setType(VcsJob::Apply);
    *job << "git" << "apply";
    if (applyTo == Index) {
        *job << "--index";   // Applies the diff also to the index
        *job << "--cached";  // Does not touch the work tree
    }
    auto* const diffFile = new QTemporaryFile(job);
    if (diffFile->open()) {
        *job << diffFile->fileName();
        diffFile->write(diff.diff().toUtf8());
        diffFile->close();
    } else {
        job->cancel();
        delete diffFile;
    }
    return job;
}


VcsJob* GitPlugin::revert(const QList<QUrl>& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    if(localLocations.isEmpty() )
        return makeVcsErrorJob(i18n("Could not revert changes"), this, OutputJob::Verbose);

    QDir repo = urlDir(repositoryRoot(localLocations.first()));
    QString modified;
    for (const auto& file: localLocations) {
        if (hasModifications(repo, file)) {
            modified.append(file.toDisplayString(QUrl::PreferLocalFile) + QLatin1String("<br/>"));
        }
    }
    if (!modified.isEmpty()) {
        auto res = KMessageBox::questionTwoActions(nullptr,
                                                   i18n("The following files have uncommitted changes, "
                                                        "which will be lost. Continue?")
                                                       + QLatin1String("<br/><br/>") + modified,
                                                   {}, KStandardGuiItem::discard(), KStandardGuiItem::cancel());
        if (res != KMessageBox::PrimaryAction) {
            return makeVcsErrorJob(QString(), this, OutputJob::Silent);
        }
    }

    DVcsJob* job = new GitJob(dotGitDirectory(localLocations.front()), this);
    job->setType(VcsJob::Revert);
    *job << "git" << "checkout" << "--";
    *job << (recursion == IBasicVersionControl::Recursive ? localLocations : preventRecursion(localLocations));

    return job;
}


//TODO: git doesn't like empty messages, but "KDevelop didn't provide any message, it may be a bug" looks ugly...
//If no files specified then commit already added files
VcsJob* GitPlugin::commit(const QString& message,
                             const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    if (localLocations.empty() || message.isEmpty())
        return makeVcsErrorJob(i18n("No files or message specified"), this);

    const QDir dir = dotGitDirectory(localLocations.front());
    if (!ensureValidGitIdentity(dir)) {
        return makeVcsErrorJob(i18n("Email or name for Git not specified"), this);
    }

    auto* job = new GitJob(dir, this);
    job->setType(VcsJob::Commit);
    QList<QUrl> files = (recursion == IBasicVersionControl::Recursive ? localLocations : preventRecursion(localLocations));
    addNotVersionedFiles(dir, files);

    *job << "git" << "commit" << "-m" << message;
    *job << "--" << files;
    return job;
}

KDevelop::VcsJob * GitPlugin::commitStaged(const QString& message, const QUrl& repoUrl)
{
    if (message.isEmpty())
        return makeVcsErrorJob(i18n("No message specified"), this);
    const QDir dir = dotGitDirectory(repoUrl);
    if (!ensureValidGitIdentity(dir)) {
        return makeVcsErrorJob(i18n("Email or name for Git not specified"), this);
    }
    auto* job = new GitJob(dir, this);
    job->setType(VcsJob::Commit);
    *job << "git" << "commit" << "-m" << message;
    return job;
}


bool GitPlugin::ensureValidGitIdentity(const QDir& dir)
{
    const QUrl url = QUrl::fromLocalFile(dir.absolutePath());

    const QString name = readConfigOption(url, QStringLiteral("user.name"));
    const QString email = readConfigOption(url, QStringLiteral("user.email"));
    if (!email.isEmpty() && !name.isEmpty()) {
        return true; // already okay
    }

    GitNameEmailDialog dialog;
    dialog.setName(name);
    dialog.setEmail(email);
    if (!dialog.exec()) {
        return false;
    }

    runSynchronously(setConfigOption(url, QStringLiteral("user.name"), dialog.name(), dialog.isGlobal()));
    runSynchronously(setConfigOption(url, QStringLiteral("user.email"), dialog.email(), dialog.isGlobal()));
    return true;
}

void GitPlugin::addNotVersionedFiles(const QDir& dir, const QList<QUrl>& files)
{
    const QStringList otherStr = getLsFiles(dir, QStringList() << QStringLiteral("--others"), KDevelop::OutputJob::Silent);
    QList<QUrl> toadd, otherFiles;

    otherFiles.reserve(otherStr.size());
    for (const QString& file : otherStr) {
        QUrl v = QUrl::fromLocalFile(dir.absoluteFilePath(file));

        otherFiles += v;
    }

    //We add the files that are not versioned
    for (const QUrl& file : files) {
        if(otherFiles.contains(file) && QFileInfo(file.toLocalFile()).isFile())
            toadd += file;
    }

    if(!toadd.isEmpty()) {
        VcsJob* job = add(toadd);
        job->exec(); // krazy:exclude=crashy
    }
}

bool isEmptyDirStructure(const QDir &dir)
{
    const auto infos = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QFileInfo& i : infos) {
        if (i.isDir()) {
            if (!isEmptyDirStructure(QDir(i.filePath()))) return false;
        } else if (i.isFile()) {
            return false;
        }
    }
    return true;
}

VcsJob* GitPlugin::remove(const QList<QUrl>& files)
{
    if (files.isEmpty())
        return makeVcsErrorJob(i18n("No files to remove"), this);
    QDir dotGitDir = dotGitDirectory(files.front());


    QList<QUrl> files_(files);

    QMutableListIterator<QUrl> i(files_);
    while (i.hasNext()) {
        QUrl file = i.next();
        QFileInfo fileInfo(file.toLocalFile());

        const QStringList otherStr = getLsFiles(dotGitDir, QStringList{QStringLiteral("--others"), QStringLiteral("--"), file.toLocalFile()}, KDevelop::OutputJob::Silent);
        if(!otherStr.isEmpty()) {
            //remove files not under version control
            QList<QUrl> otherFiles;
            otherFiles.reserve(otherStr.size());
            for (const QString& f : otherStr) {
                otherFiles << QUrl::fromLocalFile(dotGitDir.path() + QLatin1Char('/') + f);
            }
            if (fileInfo.isFile()) {
                //if it's an unversioned file we are done, don't use git rm on it
                i.remove();
            }

            auto deleteJob = KIO::del(otherFiles);
            deleteJob->exec();
            qCDebug(PLUGIN_GIT) << "other files" << otherFiles;
        }

        if (fileInfo.isDir()) {
            if (isEmptyDirStructure(QDir(file.toLocalFile()))) {
                //remove empty folders, git doesn't do that
                auto deleteJob = KIO::del(file);
                deleteJob->exec();
                qCDebug(PLUGIN_GIT) << "empty folder, removing" << file;
                //we already deleted it, don't use git rm on it
                i.remove();
            }
        }
    }

    if (files_.isEmpty()) return nullptr;

    DVcsJob* job = new GitJob(dotGitDir, this);
    job->setType(VcsJob::Remove);
    // git refuses to delete files with local modifications
    // use --force to overcome this
    *job << "git" << "rm" << "-r" << "--force";
    *job << "--" << files_;
    return job;
}

VcsJob* GitPlugin::log(const QUrl& localLocation,
                const KDevelop::VcsRevision& src, const KDevelop::VcsRevision& dst)
{
    DVcsJob* job = new GitJob(dotGitDirectory(localLocation), this, KDevelop::OutputJob::Silent);
    job->setType(VcsJob::Log);
    *job << "git" << "log" << "--date=raw" << "--name-status" << "-M80%" << "--follow" << "--format=medium" << "--no-decorate";
    QString rev = revisionInterval(dst, src);
    if(!rev.isEmpty())
        *job << rev;
    *job << "--" << localLocation;
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitLogOutput);
    return job;
}


VcsJob* GitPlugin::log(const QUrl& localLocation, const KDevelop::VcsRevision& rev, unsigned long int limit)
{
    DVcsJob* job = new GitJob(dotGitDirectory(localLocation), this, KDevelop::OutputJob::Silent);
    job->setType(VcsJob::Log);
    *job << "git" << "log" << "--date=raw" << "--name-status" << "-M80%" << "--follow" << "--format=medium" << "--no-decorate";
    QString revStr = toRevisionName(rev, QString());
    if(!revStr.isEmpty())
        *job << revStr;
    if(limit>0)
        *job << QStringLiteral("-%1").arg(limit);

    *job << "--" << localLocation;
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitLogOutput);
    return job;
}

KDevelop::VcsJob* GitPlugin::annotate(const QUrl &localLocation, const KDevelop::VcsRevision&)
{
    DVcsJob* job = new GitJob(dotGitDirectory(localLocation), this, KDevelop::OutputJob::Silent);
    job->setType(VcsJob::Annotate);
    *job << "git" << "blame" << "--porcelain" << "-w";
    *job << "--" << localLocation;
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitBlameOutput);
    return job;
}

void GitPlugin::parseGitBlameOutput(DVcsJob *job)
{
    QVariantList results;
    VcsAnnotationLine* annotation = nullptr;
    const auto output = job->output();
    const auto lines = QStringView{output}.split(QLatin1Char('\n'));

    bool skipNext=false;
    QHash<QString, VcsAnnotationLine> definedRevisions;
    for (const auto line : lines) {
        if(skipNext) {
            skipNext=false;
            results += QVariant::fromValue(*annotation);

            continue;
        }

        if (line.isEmpty())
            continue;

        constexpr QLatin1Char space{' '};

        auto name = line;
        QStringView value;
        if (const auto spaceIndex = line.indexOf(space); spaceIndex != -1) {
            name = line.first(spaceIndex);
            value = line.sliced(spaceIndex + 1);
        }

        if(name==QLatin1String("author"))
            annotation->setAuthor(value.toString());
        else if(name==QLatin1String("author-mail")) {} //TODO: do smth with the e-mail?
        else if(name==QLatin1String("author-tz")) {} //TODO: does it really matter?
        else if(name==QLatin1String("author-time"))
            annotation->setDate(QDateTime::fromSecsSinceEpoch(value.toUInt(), QTimeZone::LocalTime));
        else if(name==QLatin1String("summary"))
            annotation->setCommitMessage(value.toString());
        else if(name.startsWith(QLatin1String("committer"))) {} //We will just store the authors
        else if(name==QLatin1String("previous")) {} //We don't need that either
        else if(name==QLatin1String("filename")) { skipNext=true; }
        else if (name == QLatin1String("boundary")) {
            // We never limit the annotation with revision range specifiers, so the initial commit
            // in the git repository is always denoted as the boundary revision => not interesting.
        } else {
            constexpr auto revisionValueSize = 8;
            if (name.size() < revisionValueSize) {
                qCWarning(PLUGIN_GIT) << "first git-blame header line does not start with a long enough SHA-1 hash:"
                                      << line;
                continue;
            }

            const auto valueSpaceIndex = value.indexOf(space);
            if (valueSpaceIndex == -1) {
                qCWarning(PLUGIN_GIT)
                    << "first git-blame header line does not contain two line numbers separated with a space:" << line;
                continue;
            }
            value = value.sliced(valueSpaceIndex + 1); // skip the line number of the line in the original file
            value = leftOfNeedleOrEntireView(value, space);
            // value should now contain the line number of the line in the final file
            const auto lineNumber = value.toInt();
            if (lineNumber <= 0) {
                // This check detects both a number less than 1 and not-a-number, in which case toInt() returns 0.
                qCWarning(PLUGIN_GIT)
                    << "the second (one-based) line number in the first git-blame header line is invalid:" << line;
                continue;
            }

            annotation = &definedRevisions[name.toString()];
            if (annotation->revision().revisionType() == VcsRevision::Invalid) {
                // Just inserted a default-constructed annotation line => this commit has not been encountered before.
                // The following lines will contain the commit's details (author, time, summary, filename).
                VcsRevision rev;
                rev.setRevisionValue(name.first(revisionValueSize).toString(), VcsRevision::GlobalNumber);
                annotation->setRevision(std::move(rev));
            } else {
                // This commit's details have already been parsed and are stored in *annotation. The uninteresting to us
                // line of code will follow. Below we update only the line number and thus reuse the commit's details.
                skipNext = true;
            }

            // git line number is one-based but VcsAnnotationLine::lineNumber() is zero-based
            annotation->setLineNumber(lineNumber - 1);
        }
    }
    job->setResults(results);
}


DVcsJob* GitPlugin::lsFiles(const QDir &repository, const QStringList &args,
                            OutputJob::OutputJobVerbosity verbosity)
{
    auto* job = new GitJob(repository, this, verbosity);
    *job << "git" << "ls-files" << args;
    return job;
}

VcsJob* GitPlugin::gitStash(const QDir& repository, const QStringList& args, OutputJob::OutputJobVerbosity verbosity)
{
    auto* job = new GitJob(repository, this, verbosity);
    *job << "git" << "stash" << args;
    return job;
}

VcsJob* GitPlugin::stashList(const QDir& repository,
                                       KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    /* The format returns 4 fields separated by a 0-byte character (%x00):
     *
     *   %gd ... shortened reflog selector
     *   %p  ... abbreviated parent hashes (separated by a space, the first is the commit
     *                                      on which the stash was made)
     *   %s  ... subject (the stash message)
     *   %ct ... committer timestamp
     *
     * see man git-log, PRETTY FORMATS section and man git-stash for details.
     */
    auto* job=qobject_cast<DVcsJob*>(gitStash(repository, QStringList({
        QStringLiteral("list"),
        QStringLiteral("--format=format:%gd%x00%P%x00%s%x00%ct"),
    }), verbosity));
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitStashList);
    return job;
}

void GitPlugin::parseGitStashList(KDevelop::VcsJob* _job)
{
    auto* job = qobject_cast<DVcsJob*>(_job);
    const QList<QByteArray> output = job->rawOutput().split('\n');
    QList<StashItem> results;

    for (const QByteArray& line : output) {
        if (line.isEmpty()) continue;

        const QList<QByteArray> fields = line.split('\x00');

        /* Extract the fields */
        Q_ASSERT(fields.length() >= 4);
        const auto message = QString::fromUtf8(fields[2]);
        const auto parentHash = QString::fromUtf8(fields[1].split(' ')[0]);
        const auto creationTime = QDateTime::fromSecsSinceEpoch(fields[3].toInt());
        const auto shortRef = QString::fromUtf8(fields[0]);
        const auto stackDepth = fields[0].mid(7, fields[0].indexOf('}')-7).toInt();

        QStringView branch;
        QStringView parentCommitDesc;
        constexpr QLatin1String wipPrefix("WIP on ", 7);
        if (message.startsWith(wipPrefix)) {
            const QStringView messageView = message;
            const auto colonIndex = message.indexOf(QLatin1Char{':'}, wipPrefix.size());
            if (colonIndex == -1) {
                branch = messageView.sliced(wipPrefix.size());
                qCWarning(PLUGIN_GIT) << "missing ':' in a git stash message:" << message;
            } else {
                branch = messageView.sliced(wipPrefix.size(), colonIndex - wipPrefix.size());
                parentCommitDesc = slicedOrEmptyView(messageView, colonIndex + 2);
            }
        }

        results << StashItem {
            stackDepth,
            shortRef,
            parentHash,
            parentCommitDesc.toString(),
            branch.toString(),
            message,
            creationTime,
        };
    }
    job->setResults(QVariant::fromValue(results));
}

VcsJob* GitPlugin::tag(const QUrl& repository, const QString& commitMessage, const VcsRevision& rev, const QString& tagName)
{
    auto* job = new GitJob(urlDir(repository), this);
    *job << "git" << "tag" << "-m" << commitMessage << tagName;
    if(rev.revisionValue().isValid())
        *job << rev.revisionValue().toString();
    return job;
}

VcsJob* GitPlugin::switchBranch(const QUrl &repository, const QString &branch)
{
    QDir d=urlDir(repository);

    if(hasModifications(d)) {
        auto answer = KMessageBox::questionTwoActionsCancel(
            nullptr, i18n("There are pending changes, do you want to stash them first?"), {},
            KGuiItem(i18nc("@action:button", "Stash"), QStringLiteral("vcs-stash")),
            KGuiItem(i18nc("@action:button", "Keep"), QStringLiteral("dialog-cancel")));
        if (answer == KMessageBox::PrimaryAction) {
            QScopedPointer<VcsJob> stash(gitStash(d, QStringList(), KDevelop::OutputJob::Verbose));
            stash->exec();
        } else if (answer == KMessageBox::Cancel) {
            return nullptr;
        }
    }

    auto* job = new GitJob(d, this);
    *job << "git" << "checkout" << branch;
    return job;
}

VcsJob* GitPlugin::branch(const QUrl& repository, const KDevelop::VcsRevision& rev, const QString& branchName)
{
    Q_ASSERT(!branchName.isEmpty());

    auto* job = new GitJob(urlDir(repository), this);
    *job << "git" << "branch" << "--" << branchName;

    if(rev.revisionType() == VcsRevision::Special && rev.specialType() == VcsRevision::Head) {
        *job << "HEAD";
    } else if(!rev.prettyValue().isEmpty()) {
        *job << rev.revisionValue().toString();
    }
    return job;
}

VcsJob* GitPlugin::deleteBranch(const QUrl& repository, const QString& branchName)
{
    auto* job = new GitJob(urlDir(repository), this, OutputJob::Silent);
    *job << "git" << "branch" << "-D" << branchName;
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitCurrentBranch);
    return job;
}

VcsJob* GitPlugin::renameBranch(const QUrl& repository, const QString& oldBranchName, const QString& newBranchName)
{
    auto* job = new GitJob(urlDir(repository), this, OutputJob::Silent);
    *job << "git" << "branch" << "-m" << newBranchName << oldBranchName;
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitCurrentBranch);
    return job;
}

VcsJob* GitPlugin::mergeBranch(const QUrl& repository, const QString& branchName)
{
    Q_ASSERT(!branchName.isEmpty());

    auto* job = new GitJob(urlDir(repository), this);
    *job << "git" << "merge" << branchName;

    return job;
}

VcsJob* GitPlugin::rebase(const QUrl& repository, const QString& branchName)
{
    auto* job = new GitJob(urlDir(repository), this);
    *job << "git" << "rebase" << branchName;

    return job;
}

VcsJob* GitPlugin::currentBranch(const QUrl& repository)
{
    auto* job = new GitJob(urlDir(repository), this, OutputJob::Silent);
    job->setIgnoreError(true);
    *job << "git" << "symbolic-ref" << "-q" << "--short" << "HEAD";
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitCurrentBranch);
    return job;
}

void GitPlugin::parseGitCurrentBranch(DVcsJob* job)
{
    QString out = job->output().trimmed();

    job->setResults(out);
}

VcsJob* GitPlugin::branches(const QUrl &repository)
{
    auto* job = new GitJob(urlDir(repository));
    *job << "git" << "branch" << "-a";
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitBranchOutput);
    return job;
}

void GitPlugin::parseGitBranchOutput(DVcsJob* job)
{
    const auto output = job->output();
    const auto branchListDirty = QStringView{output}.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    QStringList branchList;
    for (auto branch : branchListDirty) {
        // Skip pointers to another branches (one example of this is "origin/HEAD -> origin/master");
        // "git rev-list" chokes on these entries and we do not need duplicate branches altogether.
        if (branch.contains(QLatin1String("->")))
            continue;

        // Skip entries such as '(no branch)'
        if (branch.contains(QLatin1String("(no branch)")))
            continue;

        if (branch.startsWith(QLatin1Char{'*'})) {
            branch = branch.sliced(1);
        }
        branchList << branch.trimmed().toString();
    }

    job->setResults(branchList);
}

/* Few words about how this hardcore works:
1. get all commits (with --parents)
2. select master (root) branch and get all unique commits for branches (git-rev-list br2 ^master ^br3)
3. parse allCommits. While parsing set mask (columns state for every row) for BRANCH, INITIAL, CROSS,
   MERGE and INITIAL are also set in DVCScommit::setParents (depending on parents count)
   another setType(INITIAL) is used for "bottom/root/first" commits of branches
4. find and set merges, HEADS. It's an iteration through all commits.
    - first we check if parent is from the same branch, if no then we go through all commits searching parent's index
      and set CROSS/HCROSS for rows (in 3 rows are set EMPTY after commit with parent from another tree met)
    - then we check branchesShas[i][0] to mark heads

4 can be a separate function. TODO: All this porn require refactoring (rewriting is better)!

It's a very dirty implementation.
FIXME:
1. HEAD which is head has extra line to connect it with further commit
2. If you merge branch2 to master, only new commits of branch2 will be visible (it's fine, but there will be
extra merge rectangle in master. If there are no extra commits in branch2, but there are another branches, then the place for branch2 will be empty (instead of be used for branch3).
3. Commits that have additional commit-data (not only history merging, but changes to fix conflicts) are shown incorrectly
*/

QVector<DVcsEvent> GitPlugin::allCommits(const QString& repo)
{
    initBranchHash(repo);

    const QStringList args{
        QStringLiteral("--all"),
        QStringLiteral("--pretty"),
        QStringLiteral("--parents"),
    };
    QScopedPointer<DVcsJob> job(gitRevList(repo, args));
    bool ret = job->exec();
    Q_ASSERT(ret && job->status()==VcsJob::JobSucceeded && "TODO: provide a fall back in case of failing");
    Q_UNUSED(ret);
    const QStringList commits = job->output().split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    static const QRegularExpression rx_com(QStringLiteral("commit [[:alnum:]]{40}"));

    QVector<DVcsEvent> commitList;
    DVcsEvent item;

    //used to keep where we have empty/cross/branch entry
    //true if it's an active branch (then cross or branch) and false if not
    QVector<bool> additionalFlags(branchesShas.count());
    additionalFlags.fill(false);

    //parse output
    for(int i = 0; i < commits.count(); ++i)
    {
        if (commits[i].contains(rx_com))
        {
            qCDebug(PLUGIN_GIT) << "commit found in " << commits[i];
            item.setCommit(commits[i].section(QLatin1Char(' '), 1, 1).trimmed());
//             qCDebug(PLUGIN_GIT) << "commit is: " << commits[i].section(' ', 1);

            QStringList parents;
            QString parent = commits[i].section(QLatin1Char(' '), 2);
            int section = 2;
            while (!parent.isEmpty())
            {
                /*                qCDebug(PLUGIN_GIT) << "Parent is: " << parent;*/
                parents.append(parent.trimmed());
                section++;
                parent = commits[i].section(QLatin1Char(' '), section);
            }
            item.setParents(parents);
            ++i;

            //Avoid Merge string
            while (!commits[i].startsWith(QLatin1String("Author: ")))
                    ++i;

            item.setAuthor(commits[i].section(QStringLiteral("Author: "), 1).trimmed());
//             qCDebug(PLUGIN_GIT) << "author is: " << commits[i].section("Author: ", 1);

            item.setDate(commits[++i].section(QStringLiteral("Date:   "), 1).trimmed());
//             qCDebug(PLUGIN_GIT) << "date is: " << commits[i].section("Date:   ", 1);

            QString log;
            i++; //next line!
            while (i < commits.count() && !commits[i].contains(rx_com))
                log += commits[i++];
            --i; //while took commit line
            item.setLog(log.trimmed());
//             qCDebug(PLUGIN_GIT) << "log is: " << log;

            //mask is used in CommitViewDelegate to understand what we should draw for each branch
            QList<int> mask;
            mask.reserve(branchesShas.count());

            //set mask (properties for each graph column in row)
            for(int i = 0; i < branchesShas.count(); ++i)
            {
                qCDebug(PLUGIN_GIT)<<"commit: " << item.commit();
                if (branchesShas[i].contains(item.commit()))
                {
                    mask.append(item.type()); //we set type in setParents

                    //check if parent from the same branch, if not then we have found a root of the branch
                    //and will use empty column for all further (from top to bottom) revisions
                    //FIXME: we should set CROSS between parent and child (and do it when find merge point)
                    additionalFlags[i] = false;
                    const auto parentShas = item.parents();
                    for (const QString& sha : parentShas) {
                        if (branchesShas[i].contains(sha))
                            additionalFlags[i] = true;
                    }
                    if (additionalFlags[i] == false)
                       item.setType(DVcsEvent::INITIAL); //hasn't parents from the same branch, used in drawing
                }
                else
                {
                    if (additionalFlags[i] == false)
                        mask.append(DVcsEvent::EMPTY);
                    else
                        mask.append(DVcsEvent::CROSS);
                }
                qCDebug(PLUGIN_GIT) << "mask " << i << "is " << mask[i];
            }
            item.setProperties(mask);
            commitList.append(item);
        }
    }

    //find and set merges, HEADS, require refactoring!
    for (auto iter = commitList.begin();
        iter != commitList.end(); ++iter)
    {
        QStringList parents = iter->parents();
        //we need only only child branches
        if (parents.count() != 1)
            break;

        QString parent = parents[0];
        const QString commit = iter->commit();
        bool parent_checked = false;
        int heads_checked = 0;

        for(int i = 0; i < branchesShas.count(); ++i)
        {
            //check parent
            if (branchesShas[i].contains(commit))
            {
                if (!branchesShas[i].contains(parent))
                {
                    //parent and child are not in same branch
                    //since it is list, than parent has i+1 index
                    //set CROSS and HCROSS
                    for (auto f_iter = iter;
                        f_iter != commitList.end(); ++f_iter)
                    {
                        if (parent == f_iter->commit())
                        {
                            for(int j = 0; j < i; ++j)
                            {
                                if(branchesShas[j].contains(parent))
                                    f_iter->setProperty(j, DVcsEvent::MERGE);
                                else
                                    f_iter->setProperty(j, DVcsEvent::HCROSS);
                            }
                            f_iter->setType(DVcsEvent::MERGE);
                            f_iter->setProperty(i, DVcsEvent::MERGE_RIGHT);
                            qCDebug(PLUGIN_GIT) << parent << " is parent of " << commit;
                            qCDebug(PLUGIN_GIT) << f_iter->commit() << " is merge";
                            parent_checked = true;
                            break;
                        }
                        else
                            f_iter->setProperty(i, DVcsEvent::CROSS);
                    }
                }
            }
            //mark HEADs

            if (!branchesShas[i].empty() && commit == branchesShas[i][0])
            {
                iter->setType(DVcsEvent::HEAD);
                iter->setProperty(i, DVcsEvent::HEAD);
                heads_checked++;
                qCDebug(PLUGIN_GIT) << "HEAD found";
            }
            //some optimization
            if (heads_checked == branchesShas.count() && parent_checked)
                break;
        }
    }

    return commitList;
}

void GitPlugin::initBranchHash(const QString &repo)
{
    const QUrl repoUrl = QUrl::fromLocalFile(repo);
    const QStringList gitBranches = runSynchronously(branches(repoUrl)).toStringList();
    qCDebug(PLUGIN_GIT) << "BRANCHES: " << gitBranches;
    //Now root branch is the current branch. In future it should be the longest branch
    //other commitLists are got with git-rev-lits branch ^br1 ^ br2
    QString root = runSynchronously(currentBranch(repoUrl)).toString();
    QScopedPointer<DVcsJob> job(gitRevList(repo, QStringList(root)));
    bool ret = job->exec();
    Q_ASSERT(ret && job->status()==VcsJob::JobSucceeded && "TODO: provide a fall back in case of failing");
    Q_UNUSED(ret);
    const QStringList commits = job->output().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    //     qCDebug(PLUGIN_GIT) << "\n\n\n commits" << commits << "\n\n\n";
    branchesShas.append(commits);
    for (const QString& branch : gitBranches) {
        if (branch == root)
            continue;
        QStringList args(branch);
        for (const QString& branch_arg : gitBranches) {
            if (branch_arg != branch)
                //man gitRevList for '^'
                args << QLatin1Char('^') + branch_arg;
        }
        QScopedPointer<DVcsJob> job(gitRevList(repo, args));
        bool ret = job->exec();
        Q_ASSERT(ret && job->status()==VcsJob::JobSucceeded && "TODO: provide a fall back in case of failing");
        Q_UNUSED(ret);
        const QStringList commits = job->output().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        //         qCDebug(PLUGIN_GIT) << "\n\n\n commits" << commits << "\n\n\n";
        branchesShas.append(commits);
    }
}

//Actually we can just copy the output without parsing. So it's a kind of draft for future
void GitPlugin::parseLogOutput(const DVcsJob* job, QVector<DVcsEvent>& commits) const
{
//     static QRegExp rx_sep( "[-=]+" );
//     static QRegExp rx_date( "date:\\s+([^;]*);\\s+author:\\s+([^;]*).*" );

    static QRegularExpression rx_com( QStringLiteral("commit \\w{1,40}") );

    const auto output = job->output();
    const auto lines = QStringView{output}.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    DVcsEvent item;
    QString commitLog;

    for (int i=0; i<lines.count(); ++i) {
//         qCDebug(PLUGIN_GIT) << "line:" << s;
        if (rx_com.match(lines[i]).hasMatch()) {
//             qCDebug(PLUGIN_GIT) << "MATCH COMMIT";
            item.setCommit(lines[++i].toString());
            item.setAuthor(lines[++i].toString());
            item.setDate(lines[++i].toString());
            item.setLog(commitLog);
            commits.append(item);
        }
        else
        {
            //FIXME: add this in a loop to the if, like in getAllCommits()
            commitLog += lines[i] + QLatin1Char('\n');
        }
    }
}

VcsItemEvent::Actions actionsFromString(char c)
{
    switch(c) {
        case 'A': return VcsItemEvent::Added;
        case 'D': return VcsItemEvent::Deleted;
        case 'R': return VcsItemEvent::Replaced;
        case 'M': return VcsItemEvent::Modified;
    }
    return VcsItemEvent::Modified;
}

void GitPlugin::parseGitLogOutput(DVcsJob * job)
{
    static QRegExp commitRegex(QStringLiteral("^commit (\\w{8})\\w{32}"));
    static QRegExp infoRegex(QStringLiteral("^(\\w+):(.*)"));
    static QRegExp modificationsRegex(QStringLiteral("^([A-Z])[0-9]*\t([^\t]+)\t?(.*)"), Qt::CaseSensitive, QRegExp::RegExp2);
    //R099    plugins/git/kdevgit.desktop     plugins/git/kdevgit.desktop.cmake
    //M       plugins/grepview/CMakeLists.txt

    QList<QVariant> commits;

    QString contents = job->output();
    // check if git-log returned anything
    if (contents.isEmpty()) {
        job->setResults(commits); // empty list
        return;
    }

    // start parsing the output
    QTextStream s(&contents);

    VcsEvent item;
    QString message;
    bool pushCommit = false;

    while (!s.atEnd()) {
        const auto line = s.readLine();

        if (commitRegex.exactMatch(line)) {
            if (pushCommit) {
                item.setMessage(message.trimmed());
                commits.append(QVariant::fromValue(item));
                item.setItems(QList<VcsItemEvent>());
            } else {
                pushCommit = true;
            }
            VcsRevision rev;
            rev.setRevisionValue(commitRegex.cap(1), KDevelop::VcsRevision::GlobalNumber);
            item.setRevision(rev);
            message.clear();
        } else if (infoRegex.exactMatch(line)) {
            QString cap1 = infoRegex.cap(1);
            if (cap1 == QLatin1String("Author")) {
                item.setAuthor(infoRegex.cap(2).trimmed());
            } else if (cap1 == QLatin1String("Date")) {
                item.setDate(QDateTime::fromSecsSinceEpoch(
                    infoRegex.cap(2).trimmed().split(QLatin1Char(' '))[0].toUInt(), QTimeZone::LocalTime));
            }
        } else if (modificationsRegex.exactMatch(line)) {
            VcsItemEvent::Actions a = actionsFromString(modificationsRegex.cap(1).at(0).toLatin1());
            QString filenameA = modificationsRegex.cap(2);

            VcsItemEvent itemEvent;
            itemEvent.setActions(a);
            itemEvent.setRepositoryLocation(filenameA);
            if(a==VcsItemEvent::Replaced) {
                QString filenameB = modificationsRegex.cap(3);
                itemEvent.setRepositoryCopySourceLocation(filenameB);
            }

            item.addItem(itemEvent);
        } else if (line.startsWith(QLatin1String("    "))) {
            message += QStringView{line}.sliced(4) + QLatin1Char('\n');
        }
    }

    item.setMessage(message.trimmed());
    commits.append(QVariant::fromValue(item));
    job->setResults(commits);
}

void GitPlugin::parseGitDiffOutput(DVcsJob* job)
{
    VcsDiff diff;
    diff.setDiff(job->output());
    diff.setBaseDiff(repositoryRoot(QUrl::fromLocalFile(job->directory().absolutePath())));
    diff.setDepth(usePrefix()? 1 : 0);

    job->setResults(QVariant::fromValue(diff));
}

static VcsStatusInfo::State lsfilesToState(char id)
{
    switch(id) {
        case 'H': return VcsStatusInfo::ItemUpToDate; //Cached
        case 'S': return VcsStatusInfo::ItemUpToDate; //Skip work tree
        case 'M': return VcsStatusInfo::ItemHasConflicts; //unmerged
        case 'R': return VcsStatusInfo::ItemDeleted; //removed/deleted
        case 'C': return VcsStatusInfo::ItemModified; //modified/changed
        case 'K': return VcsStatusInfo::ItemDeleted; //to be killed
        case '?': return VcsStatusInfo::ItemUnknown; //other
    }
    Q_ASSERT(false);
    return VcsStatusInfo::ItemUnknown;
}

void GitPlugin::parseGitStatusOutput_old(DVcsJob* job)
{
    const QString output = job->output();
    const auto outputLines = QStringView{output}.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    QDir dir = job->directory();
    QMap<QUrl, VcsStatusInfo::State> allStatus;
    for (const auto line : outputLines) {
        if (line.size() < 2) {
            qCWarning(PLUGIN_GIT) << "a git-ls-files output line is shorter than expected:" << line;
            continue;
        }

        VcsStatusInfo::State status = lsfilesToState(line[0].toLatin1());

        const auto url = QUrl::fromLocalFile(dir.absoluteFilePath(line.sliced(2).toString()));

        allStatus[url] = status;
    }

    QVariantList statuses;
    statuses.reserve(allStatus.size());
    QMap< QUrl, VcsStatusInfo::State >::const_iterator it = allStatus.constBegin(), itEnd=allStatus.constEnd();
    for(; it!=itEnd; ++it) {

        VcsStatusInfo status;
        status.setUrl(it.key());
        status.setState(it.value());

        statuses.append(QVariant::fromValue<VcsStatusInfo>(status));
    }

    job->setResults(statuses);
}

void GitPlugin::parseGitStatusOutput(DVcsJob* job)
{
    const auto output = job->output();
    const auto outputLines = QStringView{output}.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QDir workingDir = job->directory();
    QDir dotGit = dotGitDirectory(QUrl::fromLocalFile(workingDir.absolutePath()));

    QVariantList statuses;
    QList<QUrl> processedFiles;

    for (const auto line : outputLines) {
        //every line is 2 chars for the status, 1 space then the file desc
        if (line.size() < 3) {
            qCWarning(PLUGIN_GIT) << "a git-status --porcelain output line is shorter than expected:" << line;
            continue;
        }
        auto curr = line.sliced(3);
        const auto state = line.first(2);

        int arrow = curr.indexOf(QLatin1String(" -> "));
        if(arrow>=0) {
            VcsStatusInfo status;
            status.setUrl(QUrl::fromLocalFile(dotGit.absoluteFilePath(curr.first(arrow).toString())));
            status.setState(VcsStatusInfo::ItemDeleted);
            statuses.append(QVariant::fromValue<VcsStatusInfo>(status));
            processedFiles += status.url();

            curr = curr.sliced(arrow + 4);
        }

        constexpr QLatin1Char doubleQuote{'"'};
        if (curr.size() >= 2 && curr.front() == doubleQuote && curr.back() == doubleQuote) {
            // the path is quoted => unquote
            curr = curr.sliced(1, curr.size() - 2);
        }

        VcsStatusInfo status;
        ExtendedState ex_state = parseGitState(state);
        status.setUrl(QUrl::fromLocalFile(dotGit.absoluteFilePath(curr.toString())));
        status.setExtendedState(ex_state);
        status.setState(extendedStateToBasic(ex_state));
        processedFiles.append(status.url());

        qCDebug(PLUGIN_GIT) << "Checking git status for " << line << curr << status.state();

        statuses.append(QVariant::fromValue<VcsStatusInfo>(status));
    }
    QStringList paths;
    QStringList oldcmd=job->dvcsCommand();
    QStringList::const_iterator it=oldcmd.constBegin()+oldcmd.indexOf(QStringLiteral("--"))+1, itEnd=oldcmd.constEnd();
    paths.reserve(oldcmd.size());
    for(; it!=itEnd; ++it)
        paths += *it;

    //here we add the already up to date files
    const QStringList files = getLsFiles(job->directory(), QStringList{QStringLiteral("-c"), QStringLiteral("--")} << paths, OutputJob::Silent);
    for (const QString& file : files) {
        QUrl fileUrl = QUrl::fromLocalFile(workingDir.absoluteFilePath(file));

        if(!processedFiles.contains(fileUrl)) {
            VcsStatusInfo status;
            status.setUrl(fileUrl);
            status.setState(VcsStatusInfo::ItemUpToDate);

            statuses.append(QVariant::fromValue<VcsStatusInfo>(status));
        }
    }
    job->setResults(statuses);
}

void GitPlugin::parseGitVersionOutput(DVcsJob* job)
{
    const auto outputString = job->output();
    const auto output = QStringView{outputString}.trimmed();
    const auto versionString = rightOfLastNeedleOrEntireView(output, QLatin1Char{' '});
    const auto minimumVersion = QVersionNumber(1, 7);
    const auto actualVersion = QVersionNumber::fromString(versionString);
    m_oldVersion = actualVersion < minimumVersion;
    qCDebug(PLUGIN_GIT) << "checking git version" << versionString << actualVersion << "against" << minimumVersion
                        << m_oldVersion;
}

QStringList GitPlugin::getLsFiles(const QDir &directory, const QStringList &args,
    KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    QScopedPointer<DVcsJob> job(lsFiles(directory, args, verbosity));
    if (job->exec() && job->status() == KDevelop::VcsJob::JobSucceeded)
        return job->output().split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    return QStringList();
}

DVcsJob* GitPlugin::gitRevParse(const QString &repository, const QStringList &args,
    KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    auto* job = new GitJob(QDir(repository), this, verbosity);
    *job << "git" << "rev-parse" << args;

    return job;
}

DVcsJob* GitPlugin::gitRevList(const QString& directory, const QStringList& args)
{
    auto* job = new GitJob(urlDir(QUrl::fromLocalFile(directory)), this, KDevelop::OutputJob::Silent);
    {
        *job << "git" << "rev-list" << args;
        return job;
    }
}

constexpr int _pair(char a, char b) { return a*256 + b;}

GitPlugin::ExtendedState GitPlugin::parseGitState(QStringView msg)
{
    Q_ASSERT(msg.size()==1 || msg.size()==2);
    ExtendedState ret = GitInvalid;

    if(msg.contains(QLatin1Char('U')) || msg == QLatin1String("AA") || msg == QLatin1String("DD"))
        ret = GitConflicts;
    else switch(_pair(msg.at(0).toLatin1(), msg.at(1).toLatin1()))
    {
        case _pair(' ', ' '):
            ret = GitXX;
            break;
        case _pair(' ','T'): // Typechange
        case _pair(' ','M'):
            ret = GitXM;
            break;
        case _pair ( ' ','D' ) :
            ret = GitXD;
            break;
        case _pair ( ' ','R' ) :
            ret = GitXR;
            break;
        case _pair ( ' ','C' ) :
            ret = GitXC;
            break;
        case _pair ( 'T',' ' ) : // Typechange
        case _pair ( 'M',' ' ) :
            ret = GitMX;
            break;
        case _pair ( 'M','M' ) :
            ret = GitMM;
            break;
        case _pair ( 'M','D' ) :
            ret = GitMD;
            break;
        case _pair ( 'A',' ' ) :
            ret = GitAX;
            break;
        case _pair ( 'A','M' ) :
            ret = GitAM;
            break;
        case _pair ( 'A','D' ) :
            ret = GitAD;
            break;
        case _pair ( 'D',' ' ) :
            ret = GitDX;
            break;
        case _pair ( 'D','R' ) :
            ret = GitDR;
            break;
        case _pair ( 'D','C' ) :
            ret = GitDC;
            break;
        case _pair ( 'R',' ' ) :
            ret = GitRX;
            break;
        case _pair ( 'R','M' ) :
            ret = GitRM;
            break;
        case _pair ( 'R','D' ) :
            ret = GitRD;
            break;
        case _pair ( 'C',' ' ) :
            ret = GitCX;
            break;
        case _pair ( 'C','M' ) :
            ret = GitCM;
            break;
        case _pair ( 'C','D' ) :
            ret = GitCD;
            break;
        case _pair ( '?','?' ) :
            ret = GitUntracked;
            break;
        default:
            qCDebug(PLUGIN_GIT) << "Git status not identified:" << msg;
            ret = GitInvalid;
            break;
    }

    return ret;
}

KDevelop::VcsStatusInfo::State GitPlugin::extendedStateToBasic(const GitPlugin::ExtendedState state)
{
    switch(state) {
        case GitXX: return VcsStatusInfo::ItemUpToDate;
        case GitXM: return VcsStatusInfo::ItemModified;
        case GitXD: return VcsStatusInfo::ItemDeleted;
        case GitXR: return VcsStatusInfo::ItemModified;
        case GitXC: return VcsStatusInfo::ItemModified;
        case GitMX: return VcsStatusInfo::ItemModified;
        case GitMM: return VcsStatusInfo::ItemModified;
        case GitMD: return VcsStatusInfo::ItemDeleted;
        case GitAX: return VcsStatusInfo::ItemAdded;
        case GitAM: return VcsStatusInfo::ItemAdded;
        case GitAD: return VcsStatusInfo::ItemAdded;
        case GitDX: return VcsStatusInfo::ItemDeleted;
        case GitDR: return VcsStatusInfo::ItemDeleted;
        case GitDC: return VcsStatusInfo::ItemDeleted;
        case GitRX: return VcsStatusInfo::ItemModified;
        case GitRM: return VcsStatusInfo::ItemModified;
        case GitRD: return VcsStatusInfo::ItemDeleted;
        case GitCX: return VcsStatusInfo::ItemModified;
        case GitCM: return VcsStatusInfo::ItemModified;
        case GitCD: return VcsStatusInfo::ItemDeleted;
        case GitUntracked: return VcsStatusInfo::ItemUnknown;
        case GitConflicts: return VcsStatusInfo::ItemHasConflicts;
        case GitInvalid: return VcsStatusInfo::ItemUnknown;
    }
    return VcsStatusInfo::ItemUnknown;
}

StandardJob::StandardJob(IPlugin* parent, KJob* job)
    // this job does not output anything itself, so pass Silent to VcsJob()
    : VcsJob(parent, OutputJob::Silent)
    , m_job(job)
    , m_plugin(parent)
    , m_status(JobNotStarted)
{}

void StandardJob::start()
{
    connect(m_job, &KJob::result, this, &StandardJob::result);
    m_job->start();
    m_status=JobRunning;
}

void StandardJob::result(KJob* job)
{
    if (job->error() == 0) {
        m_status = JobSucceeded;
        setError(NoError);
    } else {
        m_status = JobFailed;
        setError(UserDefinedError);
    }
    emitResult();
}

VcsJob* GitPlugin::copy(const QUrl& localLocationSrc, const QUrl& localLocationDstn)
{
    //TODO: Probably we should "git add" after
    return new StandardJob(this, KIO::copy(localLocationSrc, localLocationDstn));
}

VcsJob* GitPlugin::move(const QUrl& source, const QUrl& destination)
{
    QDir dir = urlDir(source);

    QFileInfo fileInfo(source.toLocalFile());
    if (fileInfo.isDir()) {
        if (isEmptyDirStructure(QDir(source.toLocalFile()))) {
            //move empty folder, git doesn't do that
            qCDebug(PLUGIN_GIT) << "empty folder" << source;
            return new StandardJob(this, KIO::move(source, destination));
        }
    }

    const QStringList otherStr = getLsFiles(dir, QStringList{QStringLiteral("--others"), QStringLiteral("--"), source.toLocalFile()}, KDevelop::OutputJob::Silent);
    if(otherStr.isEmpty()) {
        auto* job = new GitJob(dir, this, KDevelop::OutputJob::Verbose);
        *job << "git" << "mv" << source.toLocalFile() << destination.toLocalFile();
        return job;
    } else {
        return new StandardJob(this, KIO::move(source, destination));
    }
}

void GitPlugin::parseGitRepoLocationOutput(DVcsJob* job)
{
    job->setResults(QVariant::fromValue(QUrl::fromLocalFile(job->output())));
}

VcsJob* GitPlugin::repositoryLocation(const QUrl& localLocation)
{
    auto* job = new GitJob(urlDir(localLocation), this);
    //Probably we should check first if origin is the proper remote we have to use but as a first attempt it works
    *job << "git" << "config" << "remote.origin.url";
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitRepoLocationOutput);
    return job;
}

VcsJob* GitPlugin::pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const QUrl& localRepositoryLocation)
{
    auto* job = new GitJob(urlDir(localRepositoryLocation), this);
    job->setCommunicationMode(KProcess::MergedChannels);
    *job << "git" << "pull";
    if(!localOrRepoLocationSrc.localUrl().isEmpty())
        *job << localOrRepoLocationSrc.localUrl().url();
    return job;
}

VcsJob* GitPlugin::push(const QUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst)
{
    auto* job = new GitJob(urlDir(localRepositoryLocation), this);
    job->setCommunicationMode(KProcess::MergedChannels);
    *job << "git" << "push";
    if(!localOrRepoLocationDst.localUrl().isEmpty())
        *job << localOrRepoLocationDst.localUrl().url();
    return job;
}

VcsJob* GitPlugin::resolve(const QList<QUrl>& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    return add(localLocations, recursion);
}

VcsJob* GitPlugin::update(const QList<QUrl>& localLocations, const KDevelop::VcsRevision& rev, IBasicVersionControl::RecursionMode recursion)
{
    if(rev.revisionType()==VcsRevision::Special && rev.revisionValue().value<VcsRevision::RevisionSpecialType>()==VcsRevision::Head) {
        return pull(VcsLocation(), localLocations.first());
    } else {
        auto* job = new GitJob(urlDir(localLocations.first()), this);
        {
            //Probably we should check first if origin is the proper remote we have to use but as a first attempt it works
            *job << "git" << "checkout" << rev.revisionValue().toString() << "--";
            *job << (recursion == IBasicVersionControl::Recursive ? localLocations : preventRecursion(localLocations));
            return job;
        }
    }
}

void GitPlugin::setupCommitMessageEditor(const QUrl& localLocation, KTextEdit* editor) const
{
    new GitMessageHighlighter(editor);
    QFile mergeMsgFile(dotGitDirectory(localLocation).filePath(QStringLiteral(".git/MERGE_MSG")));
    // Some limit on the file size should be set since whole content is going to be read into
    // the memory. 1Mb seems to be good value since it's rather strange to have so huge commit
    // message.
    static const qint64 maxMergeMsgFileSize = 1024*1024;
    if (mergeMsgFile.size() > maxMergeMsgFileSize || !mergeMsgFile.open(QIODevice::ReadOnly))
        return;

    QString mergeMsg = QString::fromLocal8Bit(mergeMsgFile.read(maxMergeMsgFileSize));
    editor->setPlainText(mergeMsg);
}

class GitVcsLocationWidget : public KDevelop::StandardVcsLocationWidget
{
    Q_OBJECT
    public:
        explicit GitVcsLocationWidget(QWidget* parent = nullptr)
            : StandardVcsLocationWidget(parent)
        {}

        bool isCorrect() const override
        {
            return !url().isEmpty();
        }
};

KDevelop::VcsLocationWidget* GitPlugin::vcsLocation(QWidget* parent) const
{
    return new GitVcsLocationWidget(parent);
}

void GitPlugin::registerRepositoryForCurrentBranchChanges(const QUrl& repository, const QObject* listener)
{
    Q_ASSERT(listener); // precondition
    const auto headFile = gitHeadFile(repository);
    qCDebug(PLUGIN_GIT) << "registering repository" << repository.toString(QUrl::PreferLocalFile) << "and listener"
                        << listener << "with the HEAD file" << headFile;

    const auto it = findWatchedFile(headFile);
    if (it == m_watchedFiles.end()) {
        auto& watchedFile = m_watchedFiles.emplace_back(headFile);
        Q_ASSERT(watchedFile.listeners.empty());
        watchedFile.listeners.push_back(listener);

        m_watcher->addFile(headFile);
        qCDebug(PLUGIN_GIT) << "started watching the HEAD file" << headFile;
    } else {
        // headFile is already watched => just register another listener
        it->listeners.push_back(listener);
    }
}

void GitPlugin::unregisterRepositoryForCurrentBranchChanges(const QUrl& repository, const QObject* listener)
{
    Q_ASSERT(listener); // precondition
    const auto headFile = gitHeadFile(repository);
    qCDebug(PLUGIN_GIT) << "unregistering repository" << repository.toString(QUrl::PreferLocalFile) << "and listener"
                        << listener << "with the HEAD file" << headFile;

    const auto it = findWatchedFile(headFile);
    if (it == m_watchedFiles.end()) {
        qCDebug(PLUGIN_GIT) << "the HEAD file" << headFile << "is not watched => nothing to do";
        return;
    }

    // unregister listener from one of possibly multiple same-URL repositories
    it->listeners.removeOne(listener);
    // clean up
    if (it->listeners.empty()) {
        m_watchedFiles.erase(it);
        m_watcher->removeFile(headFile);
        qCDebug(PLUGIN_GIT) << "stopped watching the HEAD file" << headFile;
    }
}

void GitPlugin::fileChanged(const QString& file)
{
    const auto it = findWatchedFile(file);
    if (it == m_watchedFiles.end()) {
        // KDirWatch's internal QueuedConnection can cause this if the file just became unwatched
        qCDebug(PLUGIN_GIT) << "an unwatched file changed" << file;
        return;
    }
    qCDebug(PLUGIN_GIT) << "a watched file changed" << file;

    //We need to delay the emitted signal, otherwise the branch hasn't change yet
    //and the repository is not functional
    it->scheduleDelayedBranchChanged(this);
}

void GitPlugin::delayedBranchChanged()
{
    const auto it = std::find_if(m_watchedFiles.cbegin(), m_watchedFiles.cend(),
                                 [timer = sender()](const WatchedFile& watchedFile) {
                                     return watchedFile.isOwnTimer(timer);
                                 });
    // The timer must be present because the connection to its timeout() signal is direct and the timer is
    // destroyed synchronously along with its owner FileWatcher when an element is erased from m_watchedFiles.
    Q_ASSERT(it != m_watchedFiles.cend());
    const auto filePath = it->filePath();

    Q_ASSERT(filePath.endsWith(QLatin1String("HEAD")));
    // repository/.git/HEAD -> repository
    const auto repository = Path{filePath}.parent().parent().toUrl();
    qCDebug(PLUGIN_GIT).nospace() << "emitting repositoryBranchChanged(" << repository.toString(QUrl::PreferLocalFile)
                                  << ')';
    emit repositoryBranchChanged(repository);
}

GitPlugin::WatchedFile::WatchedFile(const QString& filePath)
    : m_filePath(filePath)
{
}

const QString& GitPlugin::WatchedFile::filePath() const
{
    return m_filePath;
}

bool GitPlugin::WatchedFile::isOwnTimer(const QObject* object) const
{
    Q_ASSERT(object);
    return m_timer.get() == object;
}

void GitPlugin::WatchedFile::scheduleDelayedBranchChanged(const GitPlugin* plugin)
{
    Q_ASSERT(plugin);

    if (!m_timer) {
        m_timer.reset(new QTimer);
        m_timer->setSingleShot(true);
        m_timer->setInterval(1000);
        m_timer->callOnTimeout(plugin, &GitPlugin::delayedBranchChanged);
    }
    // During a git rebase, multiple changes to the HEAD file can occur within the timer interval.
    // Restart the timer and emit the signal once in the end to avoid freezing the KDevelop UI.
    m_timer->start();
}

auto GitPlugin::findWatchedFile(const QString& filePath) -> std::vector<WatchedFileAndListeners>::iterator
{
    return std::find_if(m_watchedFiles.begin(), m_watchedFiles.end(), [&filePath](const WatchedFile& watchedFile) {
        return watchedFile.filePath() == filePath;
    });
}

CheckInRepositoryJob* GitPlugin::isInRepository(KTextEditor::Document* document)
{
    CheckInRepositoryJob* job = new GitPluginCheckInRepositoryJob(document, repositoryRoot(document->url()).path());
    job->start();
    return job;
}

DVcsJob* GitPlugin::setConfigOption(const QUrl& repository, const QString& key, const QString& value, bool global)
{
    auto job = new GitJob(urlDir(repository), this);
    QStringList args;
    args << QStringLiteral("git") << QStringLiteral("config");
    if(global)
        args << QStringLiteral("--global");
    args << key << value;
    *job << args;
    return job;
}

QString GitPlugin::readConfigOption(const QUrl& repository, const QString& key)
{
    QProcess exec;
    exec.setWorkingDirectory(urlDir(repository).absolutePath());
    exec.start(QStringLiteral("git"), QStringList{QStringLiteral("config"), QStringLiteral("--get"), key});
    exec.waitForFinished();
    return QString::fromUtf8(exec.readAllStandardOutput().trimmed());
}

#include "gitplugin.moc"
#include "moc_gitplugin.cpp"
