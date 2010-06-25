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

#include "gitplugin.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KLocalizedString>
#include <KAboutData>
#include <KDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>
#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include <vcs/widgets/standardvcslocationwidget.h>
#include <KIO/CopyJob>
#include "gitclonejob.h"

K_PLUGIN_FACTORY(KDevGitFactory, registerPlugin<GitPlugin>(); )
K_EXPORT_PLUGIN(KDevGitFactory(KAboutData("kdevgit","kdevgit",ki18n("Git"),"0.1",ki18n("A plugin to support git version control systems"), KAboutData::License_GPL)))

using namespace KDevelop;

QDir dotGitDirectory(const KUrl& dirPath)
{
    const QString initialPath(dirPath.toLocalFile(KUrl::RemoveTrailingSlash));
    const QFileInfo finfo(initialPath);
    QDir dir;
    if (finfo.isFile()) {
        dir = finfo.absoluteDir();
    } else {
        dir = QDir(initialPath);
        dir.makeAbsolute();
    }
    
    static const QString gitDir(".git");
    while (!dir.cd(gitDir) && dir.cdUp()) {} // cdUp, until there is a sub-directory called .git
    return dir;
}

GitPlugin::GitPlugin( QObject *parent, const QVariantList & )
    : DistributedVersionControlPlugin(parent, KDevGitFactory::componentData())
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDistributedVersionControl )

    QString EasterEgg = i18n("Thanks for the translation! Have a nice day, mr. translator!");
    Q_UNUSED(EasterEgg)

    core()->uiController()->addToolView(i18n("Git"), dvcsViewFactory());
    setXMLFile("kdevgit.rc");
}

GitPlugin::~GitPlugin()
{}

DVcsJob* GitPlugin::errorsFound(const QString& error, KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* j = new DVcsJob(this, verbosity);
    *j << "echo" << i18n("error: %1", error) << "-n";
    return j;
}

void GitPlugin::unload()
{
    core()->uiController()->removeToolView( dvcsViewFactory() );
}


QString GitPlugin::name() const
{
    return QLatin1String("Git");
}

bool GitPlugin::isValidDirectory(const KUrl & dirPath)
{
    KUrl possibleRepoRoot = m_lastRepoRoot;
    if (!m_lastRepoRoot.isValid() || !m_lastRepoRoot.isParentOf(dirPath)) {
        QDir dir=dotGitDirectory(dirPath);

        if (".git" != dir.dirName()) {  // We didn't find .git, so no need to call git
            kDebug() << "Dir:" << dirPath << " is not inside work tree of git \"" << dirPath << '"';
            return false;
        }
        dir.cdUp();
        possibleRepoRoot.setDirectory(dir.absolutePath());
    }

    // We might have found a valid repository, call git to verify it
    KDevelop::VcsJob* job = gitRevParse(possibleRepoRoot.toLocalFile(), QStringList(QString("--is-inside-work-tree")), KDevelop::OutputJob::Silent);

    if (!job) {
        kDebug() << "Failed creating job";
        return false;
    }

    job->exec();
    if (job->status() == KDevelop::VcsJob::JobSucceeded) {
        kDebug() << "Dir:" << dirPath << " is inside work tree of git (" << possibleRepoRoot << ')';
        m_lastRepoRoot = possibleRepoRoot;
        return true;
    } else if (m_lastRepoRoot == possibleRepoRoot) {   // Not a repository anymore
        m_lastRepoRoot.clear(); // Restart from scratch
        return isValidDirectory(dirPath);
    }

    return false;
}

bool GitPlugin::isVersionControlled(const KUrl &path)
{
    QFileInfo fsObject(path.toLocalFile());
    if (fsObject.isDir()) {
        return isValidDirectory(path);
    }

    QString workDir = fsObject.path();
    QString filename = fsObject.fileName();

    QStringList listfiles("--");
    listfiles.append(filename);
    QStringList otherFiles = getLsFiles(workDir, listfiles, KDevelop::OutputJob::Silent);
    return !otherFiles.empty();
}

VcsJob* GitPlugin::init(const KUrl &directory)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, directory.toLocalFile(), GitPlugin::Init) ) {
        *job << "git";
        *job << "init";
        return job;
    }
    delete job;
    return errorsFound(i18n("Could not initialize the repository"), OutputJob::Verbose);
}

VcsJob* GitPlugin::createWorkingCopy(const KDevelop::VcsLocation & source, const KUrl& dest, KDevelop::IBasicVersionControl::RecursionMode)
{
    DVcsJob* job = new GitCloneJob(this);
    if (prepareJob(job, dest.toLocalFile(), GitPlugin::Init) ) {
        *job << "git" << "clone"
             << "--progress"
             << "--" << source.localUrl().prettyUrl() << dest.toLocalFile();
        return job;
    }
    delete job;
    return errorsFound(i18n("Could not create the working copy"), OutputJob::Verbose);
}

VcsJob* GitPlugin::add(const KUrl::List& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion)
    if (localLocations.empty())
        return 0;

    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, localLocations.front().toLocalFile()) ) {
        *job << "git";
        *job << "add";
        *job << "--";
        addFileList(job, localLocations);

        return job;
    }
    delete job;
    return errorsFound(i18n("Could not add the files"), OutputJob::Verbose);
}

KDevelop::VcsJob* GitPlugin::status(const KUrl::List& localLocations,
                            KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion)
    //it's a hack!!! See VcsCommitDialog::setCommitCandidates and the usage of DVcsJob/IDVCSexecutor
    //We need results just in status, so we set them here before execution in VcsCommitDialog::setCommitCandidates
    QString repo = localLocations[0].toLocalFile();
    QList<QVariant> statuses;
    qDebug("GitPlugin::status");
    statuses << getCachedFiles(repo, KDevelop::OutputJob::Silent)
             << getModifiedFiles(repo, KDevelop::OutputJob::Silent)
             << getOtherFiles(repo, KDevelop::OutputJob::Silent);
    DVcsJob * noOp = empty_cmd(KDevelop::OutputJob::Silent);
    noOp->setResults(QVariant(statuses));
    return noOp;
}

QString toRevisionName(const KDevelop::VcsRevision& rev)
{
    switch(rev.revisionType()) {
        case VcsRevision::Special:
            switch(rev.revisionValue().value<VcsRevision::RevisionSpecialType>()) {
                case VcsRevision::Head:
                    return "^HEAD";
                case VcsRevision::Base:
                    return "HEAD";
                case VcsRevision::Working:
                    return "";
                case VcsRevision::Previous:
                case VcsRevision::Start:
                    Q_ASSERT(false && "i don't know how to do that");
            }
            break;
        case VcsRevision::GlobalNumber:
            return rev.revisionValue().toString();
        case VcsRevision::Date:
        case VcsRevision::FileNumber:
        case VcsRevision::Invalid:
            Q_ASSERT(false);
    }
    return QString();
}

VcsJob* GitPlugin::diff(const KUrl& fileOrDirectory, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision,
                        VcsDiff::Type type, IBasicVersionControl::RecursionMode recursion)
{
    //TODO: control different types
    
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, fileOrDirectory.toLocalFile()) ) {
        KUrl::List files;
        if(QFileInfo(fileOrDirectory.toLocalFile()).isDir() && recursion==IBasicVersionControl::NonRecursive)
            files = getLsFiles(fileOrDirectory.toLocalFile(), QStringList(), KDevelop::OutputJob::Silent);
        else
            files = fileOrDirectory;
        QDir dotgit=dotGitDirectory(fileOrDirectory);
        if(dotgit.cdUp())
            job->setDirectory(dotgit);
        
        *job << "git" << "diff" << "--no-prefix";
        if(dstRevision.revisionType()==VcsRevision::Special)
            *job << toRevisionName(srcRevision);
        else
            *job << toRevisionName(srcRevision)+".."+toRevisionName(dstRevision);
        *job << "--";
        addFileList(job, files);
        
        connect(job, SIGNAL(readyForParsing(DVcsJob*)), SLOT(parseGitDiffOutput(DVcsJob*)));
        return job;
    }
    
    delete job;
    return errorsFound(i18n("Could not create the patch"), OutputJob::Verbose);
}

VcsJob* GitPlugin::revert(const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    DVcsJob* job = new DVcsJob(this);
    if (!localLocations.isEmpty() && prepareJob(job, localLocations.first().toLocalFile()) ) {
        *job << "git" << "checkout";
        *job << "--";
        addFileList(job, localLocations);
        
        return job;
    }
    
    delete job;
    return errorsFound(i18n("Could not revert changes"), OutputJob::Verbose);
}


//TODO: git doesn't like empty messages, but "KDevelop didn't provide any message, it may be a bug" looks ugly...
//If no files specified then commit already added files
VcsJob* GitPlugin::commit(const QString& message,
                             const KUrl::List& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion)

    if (localLocations.empty() || message.isEmpty())
        return 0;

    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, localLocations.front().toLocalFile()) ) {
        *job << "git";
        *job << "commit";
        *job << "-m";
        //Note: the message is quoted somewhere else, so if we quote here then we have quotes in the commit log
        *job << message;
        *job << "--";
        addFileList(job, localLocations);
        return job;
    }
    delete job;
    return errorsFound(i18n("Committing failed"), OutputJob::Verbose);
}

VcsJob* GitPlugin::remove(const KUrl::List& files)
{
    if (files.empty())
        return 0;

    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, files.front().toLocalFile()) ) {
        *job << "git";
        *job << "rm";
        *job << "--";
        addFileList(job, files);
        return job;
    }
    delete job;
    return errorsFound(i18n("Could not remove the files"), OutputJob::Verbose);
}


VcsJob* GitPlugin::log(const KUrl& localLocation,
                const KDevelop::VcsRevision& rev,
                unsigned long limit)
{
    Q_UNUSED(rev)
    Q_UNUSED(limit)
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, localLocation.toLocalFile()) ) {
        *job << "git";
        *job << "log";
        *job << "--date=raw";
        *job << "--";
        addFileList(job, localLocation);
        connect(job, SIGNAL(readyForParsing(DVcsJob*)), this, SLOT(parseGitLogOutput(DVcsJob*)));
        return job;
    }
    delete job;
    return errorsFound(i18n("Could not generate the log"), OutputJob::Verbose);
}

KDevelop::VcsJob* GitPlugin::annotate(const KUrl &localLocation, const KDevelop::VcsRevision&)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, localLocation.toLocalFile()) ) {
        *job << "git";
        *job << "blame";
        *job << "--root";
        *job << "-t";
        *job << "--";
        addFileList(job, localLocation);
        connect(job, SIGNAL(readyForParsing(DVcsJob*)), this, SLOT(parseGitBlameOutput(DVcsJob*)));
    }
    delete job;
    return errorsFound(i18n("Could not read the annotations"), OutputJob::Verbose);
}

void GitPlugin::parseGitBlameOutput(DVcsJob *job)
{
    QList<QVariant> results;
    int lineNumber = 0;
    /*static? */QRegExp regex("(\\w{8}) .* ?\\((.*) (\\d+) [-+]\\d+\\s+\\d+\\)");
    foreach(const QString& line, job->output().split('\n')) {
        if (regex.indexIn(line) == 0) {
            VcsAnnotationLine annotation;
            annotation.setAuthor(regex.cap(2));
            annotation.setDate(QDateTime::fromTime_t(regex.cap(3).toUInt()));
            annotation.setLineNumber(lineNumber);
            VcsRevision rev;
            rev.setRevisionValue(regex.cap(1), KDevelop::VcsRevision::GlobalNumber);
            annotation.setRevision(rev);
            results << QVariant::fromValue(annotation);
        }
        lineNumber++;
    }
    job->setResults(results);
}

DVcsJob* GitPlugin::var(const QString & repository)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "var";
        *job << "-l";
        return job;
    }
    delete job;
    return errorsFound(i18n("Could not retrieve some repository variable"), OutputJob::Verbose);
}

DVcsJob* GitPlugin::switchBranch(const QString &repository, const QString &branch)
{
    ///TODO Check if the branch exists. or send only existed branch names here!
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "checkout";
        *job << branch;
        return job;
    }
    
    delete job;
    return errorsFound(i18n("Could not switch the branch"), OutputJob::Verbose);
}

DVcsJob* GitPlugin::branch(const QString &repository, const QString &basebranch, const QString &branch,
                             const QStringList &args)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "branch";
        //Empty branch has 'something' so it breaks the command
        if (!args.isEmpty())
            *job << args;
        *job << "--";
        if (!branch.isEmpty())
            *job << branch;
        if (!basebranch.isEmpty())
            *job << basebranch;
        return job;
    }
    delete job;
    return errorsFound(i18n("Could not create the new branch"), OutputJob::Verbose);
}

VcsJob* GitPlugin::reset(const KUrl& repository, const QStringList &args, const KUrl::List& files)
{
    if (files.empty())
        return 0;

    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, repository.toLocalFile()) ) {
        *job << "git";
        *job << "reset";
        //Empty branch has 'something' so it breaks the command
        if (!args.isEmpty())
            *job << args;
        *job << "--";
        addFileList(job, files);
        return job;
    }
    delete job;
    return errorsFound(i18n("Could not reset"), OutputJob::Verbose);
}

DVcsJob* GitPlugin::lsFiles(const QString &repository, const QStringList &args,
    KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* job = new DVcsJob(this, verbosity);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "ls-files";
        //Empty branch has 'something' so it breaks the command
        if (!args.isEmpty())
            *job << args;
        return job;
    }
    delete job;
    return errorsFound(i18n("Could not list files"), OutputJob::Verbose);
}

QString GitPlugin::curBranch(const QString &repository)
{
    QString branchName;
    DVcsJob* job = branch(repository);
    if (job)
    {
        kDebug() << "Getting branch list";
        job->exec();
        
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
            branchName = job->output();
    }

    branchName = branchName.prepend('\n').section("\n*", 1);
    branchName = branchName.section('\n', 0, 0).trimmed();
    kDebug() << "Current branch is: " << branchName;
    return branchName;
}

QStringList GitPlugin::branches(const QString &repository)
{
    QStringList branchListDirty;
    DVcsJob* job = branch(repository);
    if (job)
    {
        kDebug() << "Getting branch list";
        job->exec();
        
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
            branchListDirty = job->output().split('\n', QString::SkipEmptyParts);
        else
            return QStringList();
    }

    QStringList branchList;
    foreach(QString branch, branchListDirty)
    {
        if (branch.contains('*'))
        {
            branch = branch.prepend('\n').section("\n*", 1);
        }
        else
        {
            branch = branch.prepend('\n').section('\n', 1);
        }
        
        branch = branch.trimmed();
        branchList<<branch;
    }
    return branchList;
}

QList<QVariant> GitPlugin::getOtherFiles(const QString& directory, OutputJob::OutputJobVerbosity verbosity)
{
    QStringList otherFiles = getLsFiles(directory, QStringList(QString("--others")), verbosity );

    QList<QVariant> others;
    foreach(const QString &file, otherFiles)
    {
        VcsStatusInfo status;
        status.setUrl(stripPathToDir(directory) + file);
        status.setState(VcsStatusInfo::ItemUnknown);
        others.append(qVariantFromValue<VcsStatusInfo>(status));
    }
    return others;
}

QList<QVariant> GitPlugin::getModifiedFiles(const QString &directory, KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* job = new DVcsJob(this, verbosity);
    if (prepareJob(job, directory) )
        *job << "git";
        *job << "diff-files";
    if (job)
        job->exec();
    QStringList output;
    if (job->status() == KDevelop::VcsJob::JobSucceeded)
        output = job->output().split('\n', QString::SkipEmptyParts);
    else
        return QList<QVariant>();

    QList<QVariant> modifiedFiles;
    foreach(const QString &line, output)
    {
        KUrl file(stripPathToDir(directory) + line.section('\t', 1).trimmed());

        VcsStatusInfo status;
        status.setUrl(file);
        status.setState(charToState(line.section(' ', 4, 4)[0].toAscii()) );
        kDebug() << line[97] << " " << file.toLocalFile();

        modifiedFiles.append(qVariantFromValue<VcsStatusInfo>(status));
    }

    return modifiedFiles;
}

QList<QVariant> GitPlugin::getCachedFiles(const QString &directory, KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* job = gitRevParse(directory, QStringList(QString("--branches")), verbosity);
    job->exec();
    QStringList shaArg;
    if (job->output().isEmpty())
    {
        //there is no branches, which means there is no commit yet
        //let's create an empty tree to use with git-diff-index
        //TODO: in newer version of git (AFAIK 1.5.5) we can do:
        //"git diff-index $(git rev-parse -q --verify HEAD  || echo 4b825dc642cb6eb9a060e54bf8d69288fbee4904)"
        DVcsJob* job = new DVcsJob(this, verbosity);
        if (prepareJob(job, directory) )
        {
            *job << "git";
            *job << "mktree";
            job->setStandardInputFile("/dev/null");
        }
        if (job && job->exec() && job->status() == KDevelop::VcsJob::JobSucceeded)
            shaArg<<job->output().split('\n', QString::SkipEmptyParts);
    }
    else
        shaArg<<"HEAD";
    job = new DVcsJob(this, verbosity);
    if (prepareJob(job, directory) )
        *job << "git" << "diff-index" << "--cached" << shaArg;
    if (job)
        job->exec();
    QStringList output;
    if (job->status() == KDevelop::VcsJob::JobSucceeded)
        output = job->output().split('\n', QString::SkipEmptyParts);
    else
        return QList<QVariant>();

    QList<QVariant> cachedFiles;

    foreach(const QString &line, output)
    {
        KUrl file(stripPathToDir(directory) + line.section('\t', 1).trimmed());

        VcsStatusInfo status;
        status.setUrl(file);
        
        status.setState(VcsStatusInfo::State(charToState(line.section(' ', 4, 4)[0].toAscii() ) +
                                             VcsStatusInfo::ItemAdded) );

        kDebug() << line[97] << " " << file.toLocalFile();

        cachedFiles.append(qVariantFromValue<VcsStatusInfo>(status));
    }

    return cachedFiles;
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
    static bool hasHash = false;
    if (!hasHash)
    {
        initBranchHash(repo);
        hasHash = true;
    }
    QStringList args;
    args << "--all" << "--pretty" << "--parents";
    DVcsJob* job = gitRevList(repo, args);
    if (job)
        job->exec();
    QStringList commits = job->output().split('\n', QString::SkipEmptyParts);

    static QRegExp rx_com("commit \\w{40,40}");

    QList<DVcsEvent>commitList;
    DVcsEvent item;

    //used to keep where we have empty/cross/branch entry
    //true if it's an active branch (then cross or branch) and false if not
    QVector<bool> additionalFlags(branchesShas.count());
    foreach(int flag, additionalFlags)
        flag = false;

    //parse output
    for(int i = 0; i < commits.count(); ++i)
    {
        if (commits[i].contains(rx_com))
        {
            kDebug() << "commit found in " << commits[i];
            item.setCommit(commits[i].section(' ', 1, 1).trimmed());
//             kDebug() << "commit is: " << commits[i].section(' ', 1);

            QStringList parents;
            QString parent = commits[i].section(' ', 2);
            int section = 2;
            while (!parent.isEmpty())
            {
                /*                kDebug() << "Parent is: " << parent;*/
                parents.append(parent.trimmed());
                section++;
                parent = commits[i].section(' ', section);
            }
            item.setParents(parents);

            //Avoid Merge string
            while (!commits[i].contains("Author: "))
                    ++i;

            item.setAuthor(commits[i].section("Author: ", 1).trimmed());
//             kDebug() << "author is: " << commits[i].section("Author: ", 1);

            item.setDate(commits[++i].section("Date:   ", 1).trimmed());
//             kDebug() << "date is: " << commits[i].section("Date:   ", 1);

            QString log;
            i++; //next line!
            while (i < commits.count() && !commits[i].contains(rx_com))
                log += commits[i++];
            --i; //while took commit line
            item.setLog(log.trimmed());
//             kDebug() << "log is: " << log;

            //mask is used in CommitViewDelegate to understand what we should draw for each branch
            QList<int> mask;

            //set mask (properties for each graph column in row)
            for(int i = 0; i < branchesShas.count(); ++i)
            {
                kDebug()<<"commit: " << item.getCommit();
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
                kDebug() << "mask " << i << "is " << mask[i];
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
                            kDebug() << parent << " is parent of " << commit;
                            kDebug() << f_iter->getCommit() << " is merge";
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
                kDebug() << "HEAD found";
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
    QStringList branches = GitPlugin::branches(repo);
    kDebug() << "BRANCHES: " << branches;
    //Now root branch is the current branch. In future it should be the longest branch
    //other commitLists are got with git-rev-lits branch ^br1 ^ br2
    QString root = GitPlugin::curBranch(repo);
    DVcsJob* job = gitRevList(repo, QStringList(root));
    if (job)
        job->exec();
    QStringList commits = job->output().split('\n', QString::SkipEmptyParts);
//     kDebug() << "\n\n\n commits" << commits << "\n\n\n";
    branchesShas.append(commits);
    foreach(const QString &branch, branches)
    {
        if (branch == root)
            continue;
        QStringList args(branch);
        foreach(const QString &branch_arg, branches)
        {
            if (branch_arg != branch)
                //man gitRevList for '^'
                args<<'^' + branch_arg;
        }
        DVcsJob* job = gitRevList(repo, args);
        if (job)
            job->exec();
        QStringList commits = job->output().split('\n', QString::SkipEmptyParts);
//         kDebug() << "\n\n\n commits" << commits << "\n\n\n";
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
        kDebug() << "line:" << s ;

        if (rx_com.exactMatch(s)) {
            kDebug() << "MATCH COMMIT";
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

void GitPlugin::parseGitLogOutput(DVcsJob * job)
{
    QList<QVariant> commits;
    static QRegExp commitRegex( "^commit (\\w{8})\\w{32}" );
    static QRegExp infoRegex( "^(\\w+):(.*)" );

    QString contents = job->output();
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
    diff.setBaseDiff(KUrl(job->getDirectory().absolutePath()));
    
    job->setResults(qVariantFromValue(diff));
}

QStringList GitPlugin::getLsFiles(const QString &directory, const QStringList &args,
    KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* job = lsFiles(directory, args, verbosity);
    if (job)
    {
        job->exec();
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
            return job->output().split('\n', QString::SkipEmptyParts);
        else
            return QStringList();
    }
    return QStringList();
}

DVcsJob* GitPlugin::gitRevParse(const QString &repository, const QStringList &args,
    KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    //Use prepareJob() here only if you like "dead" recursion and KDevelop crashes
    DVcsJob* job = new DVcsJob(this, verbosity);
    QString workDir = repository;
    QFileInfo fsObject(workDir);
    if (fsObject.isFile())
        workDir = fsObject.path();

    job->clear();
    job->setDirectory(workDir);
    *job << "git";
    *job << "rev-parse";
    foreach(const QString &arg, args)
        *job << arg;

    return job;
}

DVcsJob* GitPlugin::gitRevList(const QString &repository, const QStringList &args)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "rev-list";
        foreach(const QString &arg, args)
            *job << arg;
        return job;
    }
    
    delete job;
    return errorsFound(i18n("Could not get the rev list"), OutputJob::Verbose);
}

KDevelop::VcsStatusInfo::State GitPlugin::charToState(char ch)
{
    switch (ch)
    {
        case 'M':
            return VcsStatusInfo::ItemModified;
        case 'A':
            return VcsStatusInfo::ItemAdded;
        case 'D':
            return VcsStatusInfo::ItemDeleted;
        default://TODO: hasConflicts
            return VcsStatusInfo::ItemUnknown;
    }
    return VcsStatusInfo::ItemUnknown;
}

StandardCopyJob::StandardCopyJob(IPlugin* parent, const KUrl& source, const KUrl& dest,
                                 OutputJob::OutputJobVerbosity verbosity)
    : VcsJob(parent, verbosity)
    , m_source(source), m_dest(dest)
    , m_plugin(parent)
    , m_status(JobNotStarted)
{}

void StandardCopyJob::start()
{
    KIO::CopyJob* job=KIO::copy(m_source, m_dest);
    connect(job, SIGNAL(result(KJob*)), SLOT(result(KJob*)));
    job->start();
    m_status=JobRunning;
}

void StandardCopyJob::result(KJob* job)
{
    m_status=job->error() == 0? JobSucceeded : JobFailed; emitResult();
}

VcsJob* GitPlugin::copy(const KUrl& localLocationSrc, const KUrl& localLocationDstn)
{
    //TODO: Probably we should "git add" after
    return new StandardCopyJob(this, localLocationSrc, localLocationDstn, KDevelop::OutputJob::Silent);
}

VcsJob* GitPlugin::move(const KUrl& source, const KUrl& destination)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, source.toLocalFile())) {
        *job << "git" << "mv" << source.toLocalFile() << destination.toLocalFile();
        return job;
    }
    
    delete job;
    return 0;
}

void GitPlugin::parseGitRepoLocationOutput(DVcsJob* job)
{
    job->setResults(qVariantFromValue(KUrl(job->output())));
}

VcsJob* GitPlugin::repositoryLocation(const KUrl& localLocation)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, localLocation.toLocalFile())) {
        //Probably we should check first if origin is the proper remote we have to use but as a first attempt it works
        *job << "git" << "config" << "remote.origin.url";
        connect(job, SIGNAL(readyForParsing(DVcsJob*)), SLOT(parseGitRepoLocationOutput(DVcsJob*)));
        return job;
    }
    
    delete job;
    return errorsFound(i18n("Could not get the repository location"), OutputJob::Verbose);
}

VcsJob* GitPlugin::pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const KUrl& localRepositoryLocation)
{
    return empty_cmd();
}

VcsJob* GitPlugin::push(const KUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst)
{
    return empty_cmd();
}

VcsJob* GitPlugin::resolve(const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    return empty_cmd();
}

VcsJob* GitPlugin::update(const KUrl::List& localLocations, const KDevelop::VcsRevision& rev, IBasicVersionControl::RecursionMode recursion)
{
    return empty_cmd();
}

KDevelop::VcsLocationWidget* GitPlugin::vcsLocation(QWidget* parent) const
{
    return new KDevelop::StandardVcsLocationWidget(parent);
}
