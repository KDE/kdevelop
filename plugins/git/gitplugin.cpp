/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   Copyright 2009 Hugo Parente Lima <hugo.pl@gmail.com>                  *
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
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

#include "gitplugin.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KLocalizedString>
#include <KAboutData>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QTextCodec>
#include <QDebug>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <util/path.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>
#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include <vcs/widgets/standardvcslocationwidget.h>
#include <KIO/CopyJob>
#include <KIO/NetAccess>
#include "gitclonejob.h"
#include <interfaces/contextmenuextension.h>
#include <QMenu>
#include <interfaces/iruncontroller.h>
#include "stashmanagerdialog.h"
#include <KMessageBox>
#include <KTextEdit>
#include <KDirWatch>
#include <KTextEditor/Document>
#include "gitjob.h"
#include "gitmessagehighlighter.h"
#include "gitplugincheckinrepositoryjob.h"
#include "debug.h"

Q_LOGGING_CATEGORY(PLUGIN_GIT, "kdevplatform.plugins.git")

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

QDir dotGitDirectory(const QUrl& dirPath)
{
    const QFileInfo finfo(dirPath.toLocalFile());
    QDir dir = finfo.isDir() ? QDir(finfo.filePath()): finfo.absoluteDir();

    static const QString gitDir = QStringLiteral(".git");
    while (!dir.exists(gitDir) && dir.cdUp()) {} // cdUp, until there is a sub-directory called .git

    if (dir.isRoot()) {
        qWarning() << "couldn't find the git root for" << dirPath;
    }

    return dir;
}

/**
 * Whenever a directory is provided, change it for all the files in it but not inner directories,
 * that way we make sure we won't get into recursion,
 */
static QList<QUrl> preventRecursion(const QList<QUrl>& urls)
{
    QList<QUrl> ret;
    foreach(const QUrl& url, urls) {
        QDir d(url.toLocalFile());
        if(d.exists()) {
            QStringList entries = d.entryList(QDir::Files | QDir::NoDotAndDotDot);
            foreach(const QString& entry, entries) {
                QUrl entryUrl = QUrl::fromLocalFile(d.absoluteFilePath(entry));
                ret += entryUrl;
            }
        } else
            ret += url;
    }
    return ret;
}

QString toRevisionName(const KDevelop::VcsRevision& rev, QString currentRevision=QString())
{
    switch(rev.revisionType()) {
        case VcsRevision::Special:
            switch(rev.revisionValue().value<VcsRevision::RevisionSpecialType>()) {
                case VcsRevision::Head:
                    return "^HEAD";
                case VcsRevision::Base:
                    return "";
                case VcsRevision::Working:
                    return "";
                case VcsRevision::Previous:
                    Q_ASSERT(!currentRevision.isEmpty());
                    return currentRevision + "^1";
                case VcsRevision::Start:
                    return "";
                case VcsRevision::UserSpecialType: //Not used
                    Q_ASSERT(false && "i don't know how to do that");
            }
            break;
        case VcsRevision::GlobalNumber:
            return rev.revisionValue().toString();
        case VcsRevision::Date:
        case VcsRevision::FileNumber:
        case VcsRevision::Invalid:
        case VcsRevision::UserSpecialType:
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
                ret = src+".."+dst;
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

GitPlugin::GitPlugin( QObject *parent, const QVariantList & )
    : DistributedVersionControlPlugin(parent, "kdevgit"), m_oldVersion(false)
{
    if (QStandardPaths::findExecutable("git").isEmpty()) {
        m_hasError = true;
        m_errorDescription = i18n("git is not installed");
        return;
    }

    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDistributedVersionControl )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBranchingVersionControl )

    m_hasError = false;
    setObjectName("Git");

    DVcsJob* versionJob = new DVcsJob(QDir::tempPath(), this, KDevelop::OutputJob::Silent);
    *versionJob << "git" << "--version";
    connect(versionJob, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitVersionOutput);
    ICore::self()->runController()->registerJob(versionJob);

    m_watcher = new KDirWatch(this);
    connect(m_watcher, &KDirWatch::dirty, this, &GitPlugin::fileChanged);
    connect(m_watcher, &KDirWatch::created, this, &GitPlugin::fileChanged);
}

GitPlugin::~GitPlugin()
{}

bool emptyOutput(DVcsJob* job)
{
    QScopedPointer<DVcsJob> _job(job);
    if(job->exec() && job->status()==VcsJob::JobSucceeded)
        return job->rawOutput().trimmed().isEmpty();

    return false;
}

bool GitPlugin::hasStashes(const QDir& repository)
{
    return !emptyOutput(gitStash(repository, QStringList("list"), KDevelop::OutputJob::Silent));
}

bool GitPlugin::hasModifications(const QDir& d)
{
    return !emptyOutput(lsFiles(d, QStringList("-m"), OutputJob::Silent));
}

bool GitPlugin::hasModifications(const QDir& repo, const QUrl& file)
{
    return !emptyOutput(lsFiles(repo, QStringList() << "-m" << file.path(), OutputJob::Silent));
}

void GitPlugin::additionalMenuEntries(QMenu* menu, const QList<QUrl>& urls)
{
    m_urls = urls;

    QDir dir=urlDir(urls);
    bool hasSt = hasStashes(dir);
    menu->addSeparator()->setText(i18n("Git Stashes"));
    menu->addAction(i18n("Stash Manager"), this, SLOT(ctxStashManager()))->setEnabled(hasSt);
    menu->addAction(i18n("Push Stash"), this, SLOT(ctxPushStash()));
    menu->addAction(i18n("Pop Stash"), this, SLOT(ctxPopStash()))->setEnabled(hasSt);
}

void GitPlugin::ctxPushStash()
{
    VcsJob* job = gitStash(urlDir(m_urls), QStringList(), KDevelop::OutputJob::Verbose);
    ICore::self()->runController()->registerJob(job);
}

void GitPlugin::ctxPopStash()
{
    VcsJob* job = gitStash(urlDir(m_urls), QStringList("pop"), KDevelop::OutputJob::Verbose);
    ICore::self()->runController()->registerJob(job);
}

void GitPlugin::ctxStashManager()
{
    QPointer<StashManagerDialog> d = new StashManagerDialog(urlDir(m_urls), this, 0);
    d->exec();

    delete d;
}

DVcsJob* GitPlugin::errorsFound(const QString& error, KDevelop::OutputJob::OutputJobVerbosity verbosity=OutputJob::Verbose)
{
    DVcsJob* j = new DVcsJob(QDir::temp(), this, verbosity);
    *j << "echo" << i18n("error: %1", error) << "-n";
    return j;
}

QString GitPlugin::name() const
{
    return QLatin1String("Git");
}

QUrl GitPlugin::repositoryRoot(const QUrl& path)
{
    return QUrl::fromLocalFile(dotGitDirectory(path).absolutePath());
}

bool GitPlugin::isValidDirectory(const QUrl & dirPath)
{
    QDir dir=dotGitDirectory(dirPath);

    return dir.cd(".git") && dir.exists("HEAD");
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

    QStringList otherFiles = getLsFiles(fsObject.dir(), QStringList("--") << filename, KDevelop::OutputJob::Silent);
    return !otherFiles.empty();
}

VcsJob* GitPlugin::init(const QUrl &directory)
{
    DVcsJob* job = new DVcsJob(urlDir(directory), this);
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
        return errorsFound(i18n("Did not specify the list of files"), OutputJob::Verbose);

    DVcsJob* job = new GitJob(dotGitDirectory(localLocations.front()), this);
    job->setType(VcsJob::Add);
    *job << "git" << "add" << "--" << (recursion == IBasicVersionControl::Recursive ? localLocations : preventRecursion(localLocations));
    return job;
}

KDevelop::VcsJob* GitPlugin::status(const QList<QUrl>& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    if (localLocations.empty())
        return errorsFound(i18n("Did not specify the list of files"), OutputJob::Verbose);

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
                        VcsDiff::Type /*type*/, IBasicVersionControl::RecursionMode recursion)
{
    //TODO: control different types

    DVcsJob* job = new GitJob(dotGitDirectory(fileOrDirectory), this, KDevelop::OutputJob::Silent);
    job->setType(VcsJob::Diff);
    *job << "git" << "diff" << "--no-color" << "--no-ext-diff";
    if(srcRevision.revisionType()==VcsRevision::Special
        && dstRevision.revisionType()==VcsRevision::Special
        && srcRevision.specialType()==VcsRevision::Base
        && dstRevision.specialType()==VcsRevision::Working)
        *job << "HEAD";
    else {
        QString revstr = revisionInterval(srcRevision, dstRevision);
        if(!revstr.isEmpty())
            *job << revstr;
    }

    *job << "--";
    if (recursion == IBasicVersionControl::Recursive) {
        *job << fileOrDirectory;
    } else {
        *job << preventRecursion(QList<QUrl>() << fileOrDirectory);
    }

    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitDiffOutput);
    return job;
}

VcsJob* GitPlugin::revert(const QList<QUrl>& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    if(localLocations.isEmpty() )
        return errorsFound(i18n("Could not revert changes"), OutputJob::Verbose);

    QDir repo = urlDir(repositoryRoot(localLocations.first()));
    QString modified;
    for (const auto& file: localLocations) {
        if (hasModifications(repo, file)) {
            modified.append(file.toDisplayString(QUrl::PreferLocalFile) + "<br/>");
        }
    }
    if (!modified.isEmpty()) {
        auto res = KMessageBox::questionYesNo(nullptr, i18n("The following files have uncommited changes, "
                                              "which will be lost. Continue?") + "<br/><br/>" + modified);
        if (res != KMessageBox::Yes) {
            return errorsFound(QString(), OutputJob::Silent);
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
        return errorsFound(i18n("No files or message specified"));

    QDir dir = dotGitDirectory(localLocations.front());
    DVcsJob* job = new DVcsJob(dir, this);
    job->setType(VcsJob::Commit);
    QList<QUrl> files = (recursion == IBasicVersionControl::Recursive ? localLocations : preventRecursion(localLocations));
    addNotVersionedFiles(dir, files);

    *job << "git" << "commit" << "-m" << message;
    *job << "--" << files;
    return job;
}

void GitPlugin::addNotVersionedFiles(const QDir& dir, const QList<QUrl>& files)
{
    QStringList otherStr = getLsFiles(dir, QStringList() << "--others", KDevelop::OutputJob::Silent);
    QList<QUrl> toadd, otherFiles;

    foreach(const QString& file, otherStr) {
        QUrl v = QUrl::fromLocalFile(dir.absoluteFilePath(file));

        otherFiles += v;
    }

    //We add the files that are not versioned
    foreach(const QUrl& file, files) {
        if(otherFiles.contains(file) && QFileInfo(file.toLocalFile()).isFile())
            toadd += file;
    }

    if(!toadd.isEmpty()) {
        VcsJob* job = add(toadd);
        job->exec();
    }
}

bool isEmptyDirStructure(const QDir &dir)
{
    foreach (const QFileInfo &i, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
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
        return errorsFound(i18n("No files to remove"));
    QDir dotGitDir = dotGitDirectory(files.front());


    QList<QUrl> files_(files);

    QMutableListIterator<QUrl> i(files_);
    while (i.hasNext()) {
        QUrl file = i.next();
        QFileInfo fileInfo(file.toLocalFile());

        QStringList otherStr = getLsFiles(dotGitDir, QStringList() << "--others" << "--" << file.toLocalFile(), KDevelop::OutputJob::Silent);
        if(!otherStr.isEmpty()) {
            //remove files not under version control
            QList<QUrl> otherFiles;
            foreach(const QString &f, otherStr) {
                otherFiles << QUrl::fromLocalFile(dotGitDir.path()+'/'+f);
            }
            if (fileInfo.isFile()) {
                //if it's an unversioned file we are done, don't use git rm on it
                i.remove();
            }
            KIO::NetAccess::synchronousRun(KIO::trash(otherFiles), 0);
            qCDebug(PLUGIN_GIT) << "other files" << otherFiles;
        }

        if (fileInfo.isDir()) {
            if (isEmptyDirStructure(QDir(file.toLocalFile()))) {
                //remove empty folders, git doesn't do that
                KIO::NetAccess::synchronousRun(KIO::trash(file), 0);
                qCDebug(PLUGIN_GIT) << "empty folder, removing" << file;
                //we already deleted it, don't use git rm on it
                i.remove();
            }
        }
    }

    if (files_.isEmpty()) return 0;

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
    *job << "git" << "log" << "--date=raw" << "--name-status" << "-M80%" << "--follow";
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
    *job << "git" << "log" << "--date=raw" << "--name-status" << "-M80%" << "--follow";
    QString revStr = toRevisionName(rev, QString());
    if(!revStr.isEmpty())
        *job << revStr;
    if(limit>0)
        *job << QString("-%1").arg(limit);

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
    VcsAnnotationLine* annotation = 0;
    QStringList lines = job->output().split('\n');

    bool skipNext=false;
    QMap<QString, VcsAnnotationLine> definedRevisions;
    for(QStringList::const_iterator it=lines.constBegin(), itEnd=lines.constEnd();
        it!=itEnd; ++it)
    {
        if(skipNext) {
            skipNext=false;
            results += qVariantFromValue(*annotation);

            continue;
        }

        if(it->isEmpty())
            continue;

        QString name = it->left(it->indexOf(' '));
        QString value = it->right(it->size()-name.size()-1);

        if(name=="author")
            annotation->setAuthor(value);
        else if(name=="author-mail") {} //TODO: do smth with the e-mail?
        else if(name=="author-tz") {} //TODO: does it really matter?
        else if(name=="author-time")
            annotation->setDate(QDateTime::fromTime_t(value.toUInt()));
        else if(name=="summary")
            annotation->setCommitMessage(value);
        else if(name.startsWith("committer")) {} //We will just store the authors
        else if(name=="previous") {} //We don't need that either
        else if(name=="filename") { skipNext=true; }
        else if(name=="boundary") {
            definedRevisions.insert("boundary", VcsAnnotationLine());
        }
        else
        {
            QStringList values = value.split(' ');

            VcsRevision rev;
            rev.setRevisionValue(name.left(8), KDevelop::VcsRevision::GlobalNumber);

            skipNext = definedRevisions.contains(name);

            if(!skipNext)
                definedRevisions.insert(name, VcsAnnotationLine());

            annotation = &definedRevisions[name];
            annotation->setLineNumber(values[1].toInt() - 1);
            annotation->setRevision(rev);
        }
    }
    job->setResults(results);
}


DVcsJob* GitPlugin::lsFiles(const QDir &repository, const QStringList &args,
                            OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* job = new DVcsJob(repository, this, verbosity);
    *job << "git" << "ls-files" << args;
    return job;
}

DVcsJob* GitPlugin::gitStash(const QDir& repository, const QStringList& args, OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* job = new DVcsJob(repository, this, verbosity);
    *job << "git" << "stash" << args;
    return job;
}

VcsJob* GitPlugin::tag(const QUrl& repository, const QString& commitMessage, const VcsRevision& rev, const QString& tagName)
{
    DVcsJob* job = new DVcsJob(urlDir(repository), this);
    *job << "git" << "tag" << "-m" << commitMessage << tagName;
    if(rev.revisionValue().isValid())
        *job << rev.revisionValue().toString();
    return job;
}

VcsJob* GitPlugin::switchBranch(const QUrl &repository, const QString &branch)
{
    QDir d=urlDir(repository);

    if(hasModifications(d) && KMessageBox::questionYesNo(0, i18n("There are pending changes, do you want to stash them first?"))==KMessageBox::Yes) {
        QScopedPointer<DVcsJob> stash(gitStash(d, QStringList(), KDevelop::OutputJob::Verbose));
        stash->exec();
    }

    DVcsJob* job = new DVcsJob(d, this);
    *job << "git" << "checkout" << branch;
    return job;
}

VcsJob* GitPlugin::branch(const QUrl& repository, const KDevelop::VcsRevision& rev, const QString& branchName)
{
    Q_ASSERT(!branchName.isEmpty());

    DVcsJob* job = new DVcsJob(urlDir(repository), this);
    *job << "git" << "branch" << "--" << branchName;

    if(!rev.prettyValue().isEmpty())
        *job << rev.revisionValue().toString();
    return job;
}

VcsJob* GitPlugin::deleteBranch(const QUrl& repository, const QString& branchName)
{
    DVcsJob* job = new DVcsJob(urlDir(repository), this, OutputJob::Silent);
    *job << "git" << "branch" << "-D" << branchName;
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitCurrentBranch);
    return job;
}

VcsJob* GitPlugin::renameBranch(const QUrl& repository, const QString& oldBranchName, const QString& newBranchName)
{
    DVcsJob* job = new DVcsJob(urlDir(repository), this, OutputJob::Silent);
    *job << "git" << "branch" << "-m" << newBranchName << oldBranchName;
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitCurrentBranch);
    return job;
}

VcsJob* GitPlugin::currentBranch(const QUrl& repository)
{
    DVcsJob* job = new DVcsJob(urlDir(repository), this, OutputJob::Silent);
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
    DVcsJob* job=new DVcsJob(urlDir(repository));
    *job << "git" << "branch" << "-a";
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitBranchOutput);
    return job;
}

void GitPlugin::parseGitBranchOutput(DVcsJob* job)
{
    QStringList branchListDirty = job->output().split('\n', QString::SkipEmptyParts);

    QStringList branchList;
    foreach(QString branch, branchListDirty)
    {
        // Skip pointers to another branches (one example of this is "origin/HEAD -> origin/master");
        // "git rev-list" chokes on these entries and we do not need duplicate branches altogether.
        if (branch.contains("->"))
            continue;

        // Skip entries such as '(no branch)'
        if (branch.contains("(no branch)"))
            continue;

        if (branch.startsWith('*'))
            branch = branch.right(branch.size()-2);

        branchList<<branch.trimmed();
    }

    job->setResults(branchList);
}

/* Few words about how this hardcore works:
1. get all commits (with --paretns)
2. select master (root) branch and get all unicial commits for branches (git-rev-list br2 ^master ^br3)
3. parse allCommits. While parsing set mask (columns state for every row) for BRANCH, INITIAL, CROSS,
   MERGE and INITIAL are also set in DVCScommit::setParents (depending on parents count)
   another setType(INITIAL) is used for "bottom/root/first" commits of branches
4. find and set merges, HEADS. It's an ittaration through all commits.
    - first we check if parent is from the same branch, if no then we go through all commits searching parent's index
      and set CROSS/HCROSS for rows (in 3 rows are set EMPTY after commit with parent from another tree met)
    - then we check branchesShas[i][0] to mark heads

4 can be a seporate function. TODO: All this porn require refactoring (rewriting is better)!

It's a very dirty implementation.
FIXME:
1. HEAD which is head has extra line to connect it with further commit
2. If you menrge branch2 to master, only new commits of branch2 will be visible (it's fine, but there will be
extra merge rectangle in master. If there are no extra commits in branch2, but there are another branches, then the place for branch2 will be empty (instead of be used for branch3).
3. Commits that have additional commit-data (not only history merging, but changes to fix conflicts) are shown incorrectly
*/

QList<DVcsEvent> GitPlugin::getAllCommits(const QString &repo)
{
    initBranchHash(repo);

    QStringList args;
    args << "--all" << "--pretty" << "--parents";
    QScopedPointer<DVcsJob> job(gitRevList(repo, args));
    bool ret = job->exec();
    Q_ASSERT(ret && job->status()==VcsJob::JobSucceeded && "TODO: provide a fall back in case of failing");
    Q_UNUSED(ret);
    QStringList commits = job->output().split('\n', QString::SkipEmptyParts);

    static QRegExp rx_com("commit \\w{40,40}");

    QList<DVcsEvent>commitList;
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
            item.setCommit(commits[i].section(' ', 1, 1).trimmed());
//             qCDebug(PLUGIN_GIT) << "commit is: " << commits[i].section(' ', 1);

            QStringList parents;
            QString parent = commits[i].section(' ', 2);
            int section = 2;
            while (!parent.isEmpty())
            {
                /*                qCDebug(PLUGIN_GIT) << "Parent is: " << parent;*/
                parents.append(parent.trimmed());
                section++;
                parent = commits[i].section(' ', section);
            }
            item.setParents(parents);

            //Avoid Merge string
            while (!commits[i].contains("Author: "))
                    ++i;

            item.setAuthor(commits[i].section("Author: ", 1).trimmed());
//             qCDebug(PLUGIN_GIT) << "author is: " << commits[i].section("Author: ", 1);

            item.setDate(commits[++i].section("Date:   ", 1).trimmed());
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

            //set mask (properties for each graph column in row)
            for(int i = 0; i < branchesShas.count(); ++i)
            {
                qCDebug(PLUGIN_GIT)<<"commit: " << item.getCommit();
                if (branchesShas[i].contains(item.getCommit()))
                {
                    mask.append(item.getType()); //we set type in setParents

                    //check if parent from the same branch, if not then we have found a root of the branch
                    //and will use empty column for all futher (from top to bottom) revisions
                    //FIXME: we should set CROSS between parent and child (and do it when find merge point)
                    additionalFlags[i] = false;
                    foreach(const QString &sha, item.getParents())
                    {
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
    for(QList<DVcsEvent>::iterator iter = commitList.begin();
        iter != commitList.end(); ++iter)
    {
        QStringList parents = iter->getParents();
        //we need only only child branches
        if (parents.count() != 1)
            break;

        QString parent = parents[0];
        QString commit = iter->getCommit();
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
                    for(QList<DVcsEvent>::iterator f_iter = iter;
                        f_iter != commitList.end(); ++f_iter)
                    {
                        if (parent == f_iter->getCommit())
                        {
                            for(int j = 0; j < i; ++j)
                            {
                                if(branchesShas[j].contains(parent))
                                    f_iter->setPropetry(j, DVcsEvent::MERGE);
                                else
                                    f_iter->setPropetry(j, DVcsEvent::HCROSS);
                            }
                            f_iter->setType(DVcsEvent::MERGE);
                            f_iter->setPropetry(i, DVcsEvent::MERGE_RIGHT);
                            qCDebug(PLUGIN_GIT) << parent << " is parent of " << commit;
                            qCDebug(PLUGIN_GIT) << f_iter->getCommit() << " is merge";
                            parent_checked = true;
                            break;
                        }
                        else
                            f_iter->setPropetry(i, DVcsEvent::CROSS);
                    }
                }
            }
            //mark HEADs

            if (!branchesShas[i].empty() && commit == branchesShas[i][0])
            {
                iter->setType(DVcsEvent::HEAD);
                iter->setPropetry(i, DVcsEvent::HEAD);
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
    QStringList gitBranches = runSynchronously(branches(repoUrl)).toStringList();
    qCDebug(PLUGIN_GIT) << "BRANCHES: " << gitBranches;
    //Now root branch is the current branch. In future it should be the longest branch
    //other commitLists are got with git-rev-lits branch ^br1 ^ br2
    QString root = runSynchronously(currentBranch(repoUrl)).toString();
    QScopedPointer<DVcsJob> job(gitRevList(repo, QStringList(root)));
    bool ret = job->exec();
    Q_ASSERT(ret && job->status()==VcsJob::JobSucceeded && "TODO: provide a fall back in case of failing");
    Q_UNUSED(ret);
    QStringList commits = job->output().split('\n', QString::SkipEmptyParts);
//     qCDebug(PLUGIN_GIT) << "\n\n\n commits" << commits << "\n\n\n";
    branchesShas.append(commits);
    foreach(const QString &branch, gitBranches)
    {
        if (branch == root)
            continue;
        QStringList args(branch);
        foreach(const QString &branch_arg, gitBranches)
        {
            if (branch_arg != branch)
                //man gitRevList for '^'
                args<<'^' + branch_arg;
        }
        QScopedPointer<DVcsJob> job(gitRevList(repo, args));
        bool ret = job->exec();
        Q_ASSERT(ret && job->status()==VcsJob::JobSucceeded && "TODO: provide a fall back in case of failing");
        Q_UNUSED(ret);
        QStringList commits = job->output().split('\n', QString::SkipEmptyParts);
//         qCDebug(PLUGIN_GIT) << "\n\n\n commits" << commits << "\n\n\n";
        branchesShas.append(commits);
    }
}

//Actually we can just copy the output without parsing. So it's a kind of draft for future
void GitPlugin::parseLogOutput(const DVcsJob * job, QList<DVcsEvent>& commits) const
{
//     static QRegExp rx_sep( "[-=]+" );
//     static QRegExp rx_date( "date:\\s+([^;]*);\\s+author:\\s+([^;]*).*" );

    static QRegExp rx_com( "commit \\w{1,40}" );

    QStringList lines = job->output().split('\n', QString::SkipEmptyParts);

    DVcsEvent item;
    QString commitLog;

    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];
        qCDebug(PLUGIN_GIT) << "line:" << s ;

        if (rx_com.exactMatch(s)) {
            qCDebug(PLUGIN_GIT) << "MATCH COMMIT";
            item.setCommit(s);
            s = lines[++i];
            item.setAuthor(s);
            s = lines[++i];
            item.setDate(s);
            item.setLog(commitLog);
            commits.append(item);
        }
        else
        {
            //FIXME: add this in a loop to the if, like in getAllCommits()
            commitLog += s +'\n';
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
    static QRegExp commitRegex( "^commit (\\w{8})\\w{32}" );
    static QRegExp infoRegex( "^(\\w+):(.*)" );
    static QRegExp modificationsRegex("^([A-Z])[0-9]*\t([^\t]+)\t?(.*)", Qt::CaseSensitive, QRegExp::RegExp2);
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
        QString line = s.readLine();

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
            if (cap1 == "Author") {
                item.setAuthor(infoRegex.cap(2).trimmed());
            } else if (cap1 == "Date") {
                item.setDate(QDateTime::fromTime_t(infoRegex.cap(2).trimmed().split(' ')[0].toUInt()));
            }
        } else if (modificationsRegex.exactMatch(line)) {
            VcsItemEvent::Actions a = actionsFromString(modificationsRegex.cap(1)[0].toLatin1());
            QString filenameA = modificationsRegex.cap(2);

            VcsItemEvent itemEvent;
            itemEvent.setActions(a);
            itemEvent.setRepositoryLocation(filenameA);
            if(a==VcsItemEvent::Replaced) {
                QString filenameB = modificationsRegex.cap(3);
                itemEvent.setRepositoryCopySourceLocation(filenameB);
            }

            item.addItem(itemEvent);
        } else if (line.startsWith("    ")) {
            message += line.remove(0, 4);
            message += '\n';
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

    job->setResults(qVariantFromValue(diff));
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
    QStringList outputLines = job->output().split('\n', QString::SkipEmptyParts);

    QDir dir = job->directory();
    QMap<QUrl, VcsStatusInfo::State> allStatus;
    foreach(const QString& line, outputLines) {
        VcsStatusInfo::State status = lsfilesToState(line[0].toLatin1());

        QUrl url = QUrl::fromLocalFile(dir.absoluteFilePath(line.right(line.size()-2)));

        allStatus[url] = status;
    }

    QVariantList statuses;
    QMap< QUrl, VcsStatusInfo::State >::const_iterator it = allStatus.constBegin(), itEnd=allStatus.constEnd();
    for(; it!=itEnd; ++it) {

        VcsStatusInfo status;
        status.setUrl(it.key());
        status.setState(it.value());

        statuses.append(qVariantFromValue<VcsStatusInfo>(status));
    }

    job->setResults(statuses);
}

void GitPlugin::parseGitStatusOutput(DVcsJob* job)
{
    QStringList outputLines = job->output().split('\n', QString::SkipEmptyParts);
    QDir workingDir = job->directory();
    QDir dotGit = dotGitDirectory(QUrl::fromLocalFile(workingDir.absolutePath()));

    QVariantList statuses;
    QList<QUrl> processedFiles;

    foreach(const QString& line, outputLines) {
        //every line is 2 chars for the status, 1 space then the file desc
        QString curr=line.right(line.size()-3);
        QString state = line.left(2);

        int arrow = curr.indexOf(" -> ");
        if(arrow>=0) {
            VcsStatusInfo status;
            status.setUrl(QUrl::fromLocalFile(dotGit.absoluteFilePath(curr.left(arrow))));
            status.setState(VcsStatusInfo::ItemDeleted);
            statuses.append(qVariantFromValue<VcsStatusInfo>(status));
            processedFiles += status.url();

            curr = curr.mid(arrow+4);
        }

        if(curr.startsWith('\"') && curr.endsWith('\"')) { //if the path is quoted, unquote
            curr = curr.mid(1, curr.size()-2);
        }

        VcsStatusInfo status;
        status.setUrl(QUrl::fromLocalFile(dotGit.absoluteFilePath(curr)));
        status.setState(messageToState(state));
        processedFiles.append(status.url());

        qCDebug(PLUGIN_GIT) << "Checking git status for " << line << curr << messageToState(state);

        statuses.append(qVariantFromValue<VcsStatusInfo>(status));
    }
    QStringList paths;
    QStringList oldcmd=job->dvcsCommand();
    QStringList::const_iterator it=oldcmd.constBegin()+oldcmd.indexOf("--")+1, itEnd=oldcmd.constEnd();
    for(; it!=itEnd; ++it)
        paths += *it;

    //here we add the already up to date files
    QStringList files = getLsFiles(job->directory(), QStringList() << "-c" << "--" << paths, OutputJob::Silent);
    foreach(const QString& file, files) {
        QUrl fileUrl = QUrl::fromLocalFile(workingDir.absoluteFilePath(file));

        if(!processedFiles.contains(fileUrl)) {
            VcsStatusInfo status;
            status.setUrl(fileUrl);
            status.setState(VcsStatusInfo::ItemUpToDate);

            statuses.append(qVariantFromValue<VcsStatusInfo>(status));
        }
    }
    job->setResults(statuses);
}

void GitPlugin::parseGitVersionOutput(DVcsJob* job)
{
    QStringList versionString = job->output().trimmed().split(' ').last().split('.');
    static const QList<int> minimumVersion = QList<int>() << 1 << 7;
    qCDebug(PLUGIN_GIT) << "checking git version" << versionString << "against" << minimumVersion;
    m_oldVersion = false;
    if (versionString.size() < minimumVersion.size()) {
        m_oldVersion = true;
        qWarning() << "invalid git version string:" << job->output().trimmed();
        return;
    }
    foreach(int num, minimumVersion) {
        QString curr = versionString.takeFirst();
        int valcurr = curr.toInt();
        if (valcurr < num) {
            m_oldVersion = true;
            break;
        }
        if (valcurr > num) {
            m_oldVersion = false;
            break;
        }
    }
    qCDebug(PLUGIN_GIT) << "the current git version is old: " << m_oldVersion;
}

QStringList GitPlugin::getLsFiles(const QDir &directory, const QStringList &args,
    KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    QScopedPointer<DVcsJob> job(lsFiles(directory, args, verbosity));
    if (job->exec() && job->status() == KDevelop::VcsJob::JobSucceeded)
        return job->output().split('\n', QString::SkipEmptyParts);

    return QStringList();
}

DVcsJob* GitPlugin::gitRevParse(const QString &repository, const QStringList &args,
    KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* job = new DVcsJob(QDir(repository), this, verbosity);
    *job << "git" << "rev-parse" << args;

    return job;
}

DVcsJob* GitPlugin::gitRevList(const QString& directory, const QStringList& args)
{
    DVcsJob* job = new DVcsJob(urlDir(QUrl::fromLocalFile(directory)), this, KDevelop::OutputJob::Silent);
    {
        *job << "git" << "rev-list" << args;
        return job;
    }
}

VcsStatusInfo::State GitPlugin::messageToState(const QString& msg)
{
    Q_ASSERT(msg.size()==1 || msg.size()==2);
    VcsStatusInfo::State ret = VcsStatusInfo::ItemUnknown;

    if(msg.contains('U') || msg == "AA" || msg == "DD")
        ret = VcsStatusInfo::ItemHasConflicts;
    else switch(msg[0].toLatin1())
    {
        case 'M':
            ret = VcsStatusInfo::ItemModified;
            break;
        case 'A':
            ret = VcsStatusInfo::ItemAdded;
            break;
        case 'R':
            ret = VcsStatusInfo::ItemModified;
            break;
        case 'C':
            ret = VcsStatusInfo::ItemHasConflicts;
            break;
        case ' ':
            ret = msg[1] == 'M' ? VcsStatusInfo::ItemModified : VcsStatusInfo::ItemDeleted;
            break;
        case 'D':
            ret = VcsStatusInfo::ItemDeleted;
            break;
        case '?':
            ret = VcsStatusInfo::ItemUnknown;
            break;
        default:
            qCDebug(PLUGIN_GIT) << "Git status not identified:" << msg;
            break;
    }

    return ret;
}

StandardJob::StandardJob(IPlugin* parent, KJob* job,
                                 OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, verbosity)
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
    m_status=job->error() == 0? JobSucceeded : JobFailed; emitResult();
}

VcsJob* GitPlugin::copy(const QUrl& localLocationSrc, const QUrl& localLocationDstn)
{
    //TODO: Probably we should "git add" after
    return new StandardJob(this, KIO::copy(localLocationSrc, localLocationDstn), KDevelop::OutputJob::Silent);
}

VcsJob* GitPlugin::move(const QUrl& source, const QUrl& destination)
{
    QDir dir = urlDir(source);

    QFileInfo fileInfo(source.toLocalFile());
    if (fileInfo.isDir()) {
        if (isEmptyDirStructure(QDir(source.toLocalFile()))) {
            //move empty folder, git doesn't do that
            qCDebug(PLUGIN_GIT) << "empty folder" << source;
            return new StandardJob(this, KIO::move(source, destination), KDevelop::OutputJob::Silent);
        }
    }

    QStringList otherStr = getLsFiles(dir, QStringList() << "--others" << "--" << source.toLocalFile(), KDevelop::OutputJob::Silent);
    if(otherStr.isEmpty()) {
        DVcsJob* job = new DVcsJob(dir, this, KDevelop::OutputJob::Verbose);
        *job << "git" << "mv" << source.toLocalFile() << destination.toLocalFile();
        return job;
    } else {
        return new StandardJob(this, KIO::move(source, destination), KDevelop::OutputJob::Silent);
    }
}

void GitPlugin::parseGitRepoLocationOutput(DVcsJob* job)
{
    job->setResults(QVariant::fromValue(QUrl::fromLocalFile(job->output())));
}

VcsJob* GitPlugin::repositoryLocation(const QUrl& localLocation)
{
    DVcsJob* job = new DVcsJob(urlDir(localLocation), this);
    //Probably we should check first if origin is the proper remote we have to use but as a first attempt it works
    *job << "git" << "config" << "remote.origin.url";
    connect(job, &DVcsJob::readyForParsing, this, &GitPlugin::parseGitRepoLocationOutput);
    return job;
}

VcsJob* GitPlugin::pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const QUrl& localRepositoryLocation)
{
    DVcsJob* job = new DVcsJob(urlDir(localRepositoryLocation), this);
    job->setCommunicationMode(KProcess::MergedChannels);
    *job << "git" << "-c" << "color.diff=false" << "pull";
    if(!localOrRepoLocationSrc.localUrl().isEmpty())
        *job << localOrRepoLocationSrc.localUrl().url();
    return job;
}

VcsJob* GitPlugin::push(const QUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst)
{
    DVcsJob* job = new DVcsJob(urlDir(localRepositoryLocation), this);
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
        DVcsJob* job = new DVcsJob(urlDir(localLocations.first()), this);
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
    QFile mergeMsgFile(dotGitDirectory(localLocation).filePath(".git/MERGE_MSG"));
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
    public:
        GitVcsLocationWidget(QWidget* parent = 0, Qt::WindowFlags f = 0)
            : StandardVcsLocationWidget(parent, f) {}

        virtual bool isCorrect() const
        {
            return !url().isEmpty();
        }
};

KDevelop::VcsLocationWidget* GitPlugin::vcsLocation(QWidget* parent) const
{
    return new GitVcsLocationWidget(parent);
}

bool GitPlugin::hasError() const
{
    return m_hasError;
}

QString GitPlugin::errorDescription() const
{
    return m_errorDescription;
}

void GitPlugin::registerRepositoryForCurrentBranchChanges(const QUrl& repository)
{
    QDir dir = dotGitDirectory(repository);
    QString headFile = dir.absoluteFilePath(".git/HEAD");
    m_watcher->addFile(headFile);
}

void GitPlugin::fileChanged(const QString& file)
{
    Q_ASSERT(file.endsWith("HEAD"));
    //SMTH/.git/HEAD -> SMTH/
    const QUrl fileUrl = Path(file).parent().parent().toUrl();

    //We need to delay the emitted signal, otherwise the branch hasn't change yet
    //and the repository is not functional
    m_branchesChange.append(fileUrl);
    QTimer::singleShot(1000, this, SLOT(delayedBranchChanged()));
}

void GitPlugin::delayedBranchChanged()
{
    emit repositoryBranchChanged(m_branchesChange.takeFirst());
}

CheckInRepositoryJob* GitPlugin::isInRepository(KTextEditor::Document* document)
{
    CheckInRepositoryJob* job = new GitPluginCheckInRepositoryJob(document, repositoryRoot(document->url()).path());
    job->start();
    return job;
}
