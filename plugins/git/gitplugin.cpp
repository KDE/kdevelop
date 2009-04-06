/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
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

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/dvcs/dvcsjob.h>
#include <shell/core.h>

K_PLUGIN_FACTORY(KDevGitFactory, registerPlugin<GitPlugin>(); )
K_EXPORT_PLUGIN(KDevGitFactory(KAboutData("kdevgit","kdevgit",ki18n("Git"),"0.1",ki18n("A plugin to support git version control systems"), KAboutData::License_GPL)))

using namespace KDevelop;

GitPlugin::GitPlugin( QObject *parent, const QVariantList & )
    : DistributedVersionControlPlugin(parent, KDevGitFactory::componentData())
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IDistributedVersionControl )

    QString EasterEgg = i18n("Thanks for the translation! Have a nice day, mr. translator!");
    Q_UNUSED(EasterEgg)

    core()->uiController()->addToolView(i18n("Git"), dvcsViewFactory());
    if (!(KDevelop::Core::self()->setupFlags() & KDevelop::Core::NoUi))
        setXMLFile("kdevgit.rc");
}

GitPlugin::~GitPlugin()
{
}


QString GitPlugin::name() const
{
    return QLatin1String("Git");
}

bool GitPlugin::isValidDirectory(const KUrl & dirPath)
{
    KDevelop::VcsJob* job = gitRevParse(dirPath.toLocalFile(), QStringList(QString("--is-inside-work-tree")));
    if (job)
    {
        job->exec();
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
        {
            kDebug() << "Dir:" << dirPath << " is inside work tree of git" ;
            return true;
        }
    }
    kDebug() << "Dir:" << dirPath.toLocalFile() << " is not inside work tree of git" ;
    return false;
}

bool GitPlugin::isVersionControlled(const KUrl &path)
{
    QFileInfo fsObject(path.toLocalFile());
    if (!fsObject.isFile()) {
        return isValidDirectory(path);
    }

    QString workDir = fsObject.path();
    QString filename = fsObject.fileName();

    QStringList listfiles("--");
    listfiles.append(filename);
    QStringList otherFiles = getLsFiles(workDir, listfiles);
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
    if (job) delete job;
    return NULL;
}

VcsJob* GitPlugin::createWorkingCopy(const KDevelop::VcsLocation & localOrRepoLocationSrc, const KUrl& localRepositoryRoot, KDevelop::IBasicVersionControl::RecursionMode)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, localRepositoryRoot.toLocalFile(), GitPlugin::Init) ) {
        *job << "git";
        *job << "clone";
        *job << "--";
        *job << localOrRepoLocationSrc.localUrl().pathOrUrl();
        return job;
    }
    if (job) delete job;
    return NULL;
}

VcsJob* GitPlugin::add(const KUrl::List& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion)
    if (localLocations.empty())
        return NULL;

    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, localLocations.front().toLocalFile()) ) {
        *job << "git";
        *job << "add";
        *job << "--";
        addFileList(job, localLocations);

        return job;
    }
    if (job) delete job;
    return NULL;
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
    statuses << getCachedFiles(repo)
             << getModifiedFiles(repo)
             << getOtherFiles(repo);
    DVcsJob * noOp = empty_cmd();
    noOp->setResults(QVariant(statuses));
    return noOp;
}

//TODO: git doesn't like empty messages, but "KDevelop didn't provide any message, it may be a bug" looks ugly...
//If no files specified then commit already added files
VcsJob* GitPlugin::commit(const QString& message,
                             const KUrl::List& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    Q_UNUSED(recursion)

    if (localLocations.empty() || message.isEmpty())
        return NULL;

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
    if (job) delete job;
    return NULL;
}

VcsJob* GitPlugin::remove(const KUrl::List& files)
{
    if (files.empty())
        return NULL;

    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, files.front().toLocalFile()) ) {
        *job << "git";
        *job << "rm";
        *job << "--";
        addFileList(job, files);
        return job;
    }
    if (job) delete job;
    return NULL;
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
        *job << "--";
        addFileList(job, localLocation);
        return job;
    }
    if (job) delete job;
    return NULL;
}

VcsJob* GitPlugin::log(const KUrl& localLocation,
                const KDevelop::VcsRevision& rev,
                const KDevelop::VcsRevision& limit)
{
    Q_UNUSED(limit)
    return log(localLocation, rev, 0);
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
    if (job) delete job;
    return NULL;
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
    if (job) delete job;
    return NULL;
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
    if (job) delete job;
    return NULL;
}

VcsJob* GitPlugin::reset(const KUrl& repository, const QStringList &args, const KUrl::List& files)
{
    if (files.empty())
        return NULL;

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
    if (job) delete job;
    return NULL;
}

DVcsJob* GitPlugin::lsFiles(const QString &repository, const QStringList &args)
{
    DVcsJob* job = new DVcsJob(this);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "ls-files";
        //Empty branch has 'something' so it breaks the command
        if (!args.isEmpty())
            *job << args;
        return job;
    }
    if (job) delete job;
    return NULL;
}

QString GitPlugin::curBranch(const QString &repository)
{
    DVcsJob* job = branch(repository);
    if (job)
    {
        kDebug() << "Getting branch list";
        job->exec();
    }
    QString branch;
    if (job->status() == KDevelop::VcsJob::JobSucceeded)
        branch = job->output();

    branch = branch.prepend('\n').section("\n*", 1);
    branch = branch.section('\n', 0, 0).trimmed();
    kDebug() << "Current branch is: " << branch;
    return branch;
}

QStringList GitPlugin::branches(const QString &repository)
{
    DVcsJob* job = branch(repository);
    if (job)
    {
        kDebug() << "Getting branch list";
        job->exec();
    }
    QStringList branchListDirty;
    if (job->status() == KDevelop::VcsJob::JobSucceeded)
        branchListDirty = job->output().split('\n', QString::SkipEmptyParts);
    else
        return QStringList();

    QStringList branchList;
    foreach(QString branch, branchListDirty)
    {
        if (branch.contains('*'))
        {
            branch = branch.prepend('\n').section("\n*", 1);
            branch = branch.trimmed();
        }
        else
        {
            branch = branch.prepend('\n').section('\n', 1);
            branch = branch.trimmed();
        }
        branchList<<branch;
    }
    return branchList;
}

QList<QVariant> GitPlugin::getOtherFiles(const QString &directory)
{
    QStringList otherFiles = getLsFiles(directory, QStringList(QString("--others")) );

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

QList<QVariant> GitPlugin::getModifiedFiles(const QString &directory)
{
    DVcsJob* job = new DVcsJob(this);
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
        QChar stCh = line[97];

        KUrl file(stripPathToDir(directory) + line.section('\t', 1).trimmed());

        VcsStatusInfo status;
        status.setUrl(file);
        status.setState(charToState(stCh.toAscii() ) );
        kDebug() << line[97] << " " << file.toLocalFile();

        modifiedFiles.append(qVariantFromValue<VcsStatusInfo>(status));
    }

    return modifiedFiles;
}

QList<QVariant> GitPlugin::getCachedFiles(const QString &directory)
{
    DVcsJob* job = gitRevParse(directory, QStringList(QString("--branches")));
    job->exec();
    QStringList shaArg;
    if (job->output().isEmpty())
    {
        //there is no branches, which means there is no commit yet
        //let's create an empty tree to use with git-diff-index
        //TODO: in newer version of git (AFAIK 1.5.5) we can do:
        //"git diff-index $(git rev-parse -q --verify HEAD  || echo 4b825dc642cb6eb9a060e54bf8d69288fbee4904)"
        DVcsJob* job = new DVcsJob(this);
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
    job = new DVcsJob(this);
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
        QChar stCh = line[97];

        KUrl file(stripPathToDir(directory) + line.section('\t', 1).trimmed());

        VcsStatusInfo status;
        status.setUrl(file);
        //TODO: use abother charToState or anything else! If order of constants is changed... dangerous!!!
        status.setState(VcsStatusInfo::State(charToState(stCh.toAscii() ) +
                                             VcsStatusInfo::ItemAddedIndex - VcsStatusInfo::ItemAdded) );

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

QStringList GitPlugin::getLsFiles(const QString &directory, const QStringList &args)
{
    DVcsJob* job = lsFiles(directory, args);
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

DVcsJob* GitPlugin::gitRevParse(const QString &repository, const QStringList &args)
{
    //Use prepareJob() here only if you like "dead" recursion and KDevelop crashes
    DVcsJob* job = new DVcsJob(this);
    if (job)
    {
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
    if (job) delete job;
    return NULL;
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
    if (job) delete job;
    return NULL;
}

KDevelop::VcsStatusInfo::State GitPlugin::charToState(const char ch)
{
    switch (ch)
    {
        case 'M':
        {
            return VcsStatusInfo::ItemModified;
            break;
        }
        case 'A':
        {
            return VcsStatusInfo::ItemAdded;
            break;
        }
        case 'D':
        {
            return VcsStatusInfo::ItemDeleted;
            break;
        }
        //ToDo: hasConflicts
        default:
        {
            return VcsStatusInfo::ItemUnknown;
            break;
        }
    }
    return VcsStatusInfo::ItemUnknown;
}

// #include "gitplugin.moc"
