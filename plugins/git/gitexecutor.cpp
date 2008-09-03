/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
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

#include "gitexecutor.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include <KDE/KUrl>
#include <KDE/KShell>
#include <KDE/KDebug>

#include <vcs/dvcs/dvcsjob.h>
#include <interfaces/iplugin.h>

using KDevelop::VcsStatusInfo;

GitExecutor::GitExecutor(KDevelop::IPlugin* parent)
    : QObject(parent), vcsplugin(parent)
{
}

GitExecutor::~GitExecutor()
{
}

QString GitExecutor::name() const
{
    return QLatin1String("Git");
}

//TODO: write tests for this method!
//maybe func()const?
bool GitExecutor::isValidDirectory(const KUrl & dirPath)
{
    DVCSjob* job = gitRevParse(dirPath.path(), QStringList(QString("--is-inside-work-tree")));
    if (job)
    {
        job->exec();
        if (job->status() == KDevelop::VcsJob::JobSucceeded)
        {
            kDebug() << "Dir:" << dirPath << " is inside work tree of git" ;
            return true;
        }
    }
    kDebug() << "Dir:" << dirPath.path() << " is not inside work tree of git" ;
    return false;
}

DVCSjob* GitExecutor::init(const KUrl &directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), GitExecutor::Init) ) {
        *job << "git-init";
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::clone(const KUrl &repository, const KUrl directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory.toLocalFile(), GitExecutor::Init) ) {
        *job << "git-clone";
        *job << repository.path();
//         addFileList(job, repository.path(), directory); //TODO it's temp, should work only with local repos
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::add(const QString& repository, const KUrl::List &files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "add";
        addFileList(job, files);

        return job;
    }
    if (job) delete job;
    return NULL;
}

//TODO: git doesn't like empty messages, but "KDevelop didn't provide any message, it may be a bug" looks ugly...
//If no files specified then commit already added files
DVCSjob* GitExecutor::commit(const QString& repository,
                             const QString &message, /*= "KDevelop didn't provide any message, it may be a bug"*/
                             const KUrl::List &args /*= QStringList("")*/)
{
    Q_UNUSED(args)
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-commit";
        ///In args you may find url, not only comd-line args in this case.
/*        foreach(KUrl arg, args)
            *job<<KUrl::relativeUrl(repository + QDir::separator(), arg);*/
        *job << "-m";
        //Note: the message is quoted somewhere else, so if we quote here then we have quotes in the commit log
        *job << message;
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::remove(const QString& repository, const KUrl::List &files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-rm";
        addFileList(job, files);
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::status(const QString & repository, const KUrl::List & files, bool recursive, bool taginfo)
{
    Q_UNUSED(files)
    Q_UNUSED(recursive)
    Q_UNUSED(taginfo)
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git";
        *job << "status";

        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::log(const KUrl& url)
{
    QFileInfo info(url.toLocalFile());

    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, info.absolutePath()) ) {
        *job << "git";
        *job << "log";
        *job << info.fileName();
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::var(const QString & repository)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-var";
        *job << "-l";
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::empty_cmd() const
{
    ///TODO: maybe just "" command?
    DVCSjob* job = new DVCSjob(vcsplugin);
    *job << "echo";
    *job << "-n";
    return job;
}

DVCSjob* GitExecutor::checkout(const QString &repository, const QString &branch)
{
    ///TODO Check if the branch exists. or send only existed branch names here!
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-checkout";
        *job << branch;
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::branch(const QString &repository, const QString &basebranch, const QString &branch,
                             const QStringList &args)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-branch";
        //Empty branch has 'something' so it breaks the command
        if (!args.isEmpty())
            *job << args;
        if (!branch.isEmpty())
            *job << branch;
        if (!basebranch.isEmpty())
            *job << basebranch;
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::reset(const QString &repository, const QStringList &args, const KUrl::List& files)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-reset";
        //Empty branch has 'something' so it breaks the command
        if (!args.isEmpty())
            *job << args;
        addFileList(job, files);
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::lsFiles(const QString &repository, const QStringList &args)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-ls-files";
        //Empty branch has 'something' so it breaks the command
        if (!args.isEmpty())
            *job << args;
        return job;
    }
    if (job) delete job;
    return NULL;
}

QString GitExecutor::curBranch(const QString &repository)
{
    DVCSjob* job = branch(repository);
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

QStringList GitExecutor::branches(const QString &repository)
{
    DVCSjob* job = branch(repository);
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
        if (branch.contains("*"))
        {
            branch = branch.prepend('\n').section("\n*", 1);
            branch = branch.trimmed();
        }
        else
        {
            branch = branch.prepend('\n').section("\n", 1);
            branch = branch.trimmed();
        }
        branchList<<branch;
    }
    return branchList;
}

QList<QVariant> GitExecutor::getOtherFiles(const QString &directory)
{
    QStringList otherFiles = getLsFiles(directory, QStringList(QString("--others")) );

    QList<QVariant> others;
    foreach(const QString &file, otherFiles)
    {
        VcsStatusInfo status;
        status.setUrl(directory + file);
        status.setState(VcsStatusInfo::ItemUnknown);
        others.append(qVariantFromValue<VcsStatusInfo>(status));
    }
    return others;
}

QList<QVariant> GitExecutor::getModifiedFiles(const QString &directory)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory) )
        *job << "git-diff-files";
    if (job)
        job->exec();
    QStringList output;
    if (job->status() == KDevelop::VcsJob::JobSucceeded)
        output = job->output().split('\n', QString::SkipEmptyParts);
    else
        return QList<QVariant>();

    QList<QVariant> modifiedFiles;
    foreach(QString line, output)
    {
        QChar stCh = line[97];

        KUrl file(directory + line.section('\t', 1).trimmed());

        VcsStatusInfo status;
        status.setUrl(file);
        status.setState(charToState(stCh.toAscii() ) );

        kDebug() << line[97] << " " << file.path();

        modifiedFiles.append(qVariantFromValue<VcsStatusInfo>(status));
    }

    return modifiedFiles;
}

QList<QVariant> GitExecutor::getCachedFiles(const QString &directory)
{
    DVCSjob* job = gitRevParse(directory, QStringList(QString("--branches")));
    job->exec();
    QStringList shaArg;
    if (job->output().isEmpty())
    {
        //there is no branches, which means there is no commit yet
        //let's create an empty tree to use with git-diff-index
        //TODO: in newer version of git (AFAIK 1.5.5) we can do:
        //"git diff-index $(git rev-parse -q --verify HEAD  || echo 4b825dc642cb6eb9a060e54bf8d69288fbee4904)"
        DVCSjob* job = new DVCSjob(vcsplugin);
        if (prepareJob(job, directory) )
        {
            *job << "git-mktree";
            job->setStandardInputFile("/dev/null");
        }
        if (job && job->exec() && job->status() == KDevelop::VcsJob::JobSucceeded)
            shaArg<<job->output().split('\n', QString::SkipEmptyParts);
    }
    else
        shaArg<<"HEAD";
    job = new DVCSjob(vcsplugin);
    if (prepareJob(job, directory) )
        *job << "git-diff-index" << "--cached" << shaArg;
    if (job)
        job->exec();
    QStringList output;
    if (job->status() == KDevelop::VcsJob::JobSucceeded)
        output = job->output().split('\n', QString::SkipEmptyParts);
    else
        return QList<QVariant>();

    QList<QVariant> cachedFiles;

    foreach(QString line, output)
    {
        QChar stCh = line[97];

        KUrl file(directory + line.section('\t', 1).trimmed());

        VcsStatusInfo status;
        status.setUrl(file);
        //TODO: use abother charToState or anything else! If order of constants is changed... dangerous!!!
        status.setState(VcsStatusInfo::State(charToState(stCh.toAscii() ) + 
                                             VcsStatusInfo::ItemAddedIndex - VcsStatusInfo::ItemAdded) );

        kDebug() << line[97] << " " << file.path();

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

QList<DVCScommit> GitExecutor::getAllCommits(const QString &repo)
{
    static bool hasHash = false;
    if (!hasHash)
    {
        initBranchHash(repo);
        hasHash = true;
    }
    QStringList args;
    args << "--all" << "--pretty" << "--parents";
    DVCSjob* job = gitRevList(repo, args);
    if (job)
        job->exec();
    QStringList commits = job->output().split('\n', QString::SkipEmptyParts);

    static QRegExp rx_com("commit \\w{40,40}");

    QList<DVCScommit>commitList;
    DVCScommit item;

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
                       item.setType(DVCScommit::INITIAL); //hasn't parents from the same branch, used in drawing
                }
                else
                {
                    if (additionalFlags[i] == false)
                        mask.append(DVCScommit::EMPTY);
                    else
                        mask.append(DVCScommit::CROSS);
                }
                kDebug() << "mask " << i << "is " << mask[i];
            }
            item.setProperties(mask);
            commitList.append(item);
        }
    }

    //find and set merges, HEADS, require refactoring!
    for(QList<DVCScommit>::iterator iter = commitList.begin();
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
                    for(QList<DVCScommit>::iterator f_iter = iter;
                        f_iter != commitList.end(); ++f_iter)
                    {
                        if (parent == f_iter->getCommit())
                        {
                            for(int j = 0; j < i; ++j)
                            {
                                if(branchesShas[j].contains(parent))
                                    f_iter->setPropetry(j, DVCScommit::MERGE);
                                else
                                    f_iter->setPropetry(j, DVCScommit::HCROSS);
                            }
                            f_iter->setType(DVCScommit::MERGE);
                            f_iter->setPropetry(i, DVCScommit::MERGE_RIGHT);
                            kDebug() << parent << " is parent of " << commit;
                            kDebug() << f_iter->getCommit() << " is merge";
                            parent_checked = true;
                            break;
                        }
                        else
                            f_iter->setPropetry(i, DVCScommit::CROSS);
                    }
                }
            }
            //mark HEADs
            if (commit == branchesShas[i][0])
            {
                iter->setType(DVCScommit::HEAD);
                iter->setPropetry(i, DVCScommit::HEAD);
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

void GitExecutor::initBranchHash(const QString &repo)
{
    QStringList branches = GitExecutor::branches(repo);
    kDebug() << "BRANCHES: " << branches;
    //Now root branch is the current branch. In future it should be the longest branch
    //other commitLists are got with git-rev-lits branch ^br1 ^ br2
    QString root = GitExecutor::curBranch(repo);
    DVCSjob* job = gitRevList(repo, QStringList(root));
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
        DVCSjob* job = gitRevList(repo, args);
        if (job)
            job->exec();
        QStringList commits = job->output().split('\n', QString::SkipEmptyParts);
//         kDebug() << "\n\n\n commits" << commits << "\n\n\n";
        branchesShas.append(commits);
    }
}

//Actually we can just copy the output without parsing. So it's a kind of draft for future
void GitExecutor::parseOutput(const QString& jobOutput, QList<DVCScommit>& commits) const
{
//     static QRegExp rx_sep( "[-=]+" );
//     static QRegExp rx_date( "date:\\s+([^;]*);\\s+author:\\s+([^;]*).*" );

    static QRegExp rx_com( "commit \\w{1,40}" );

    QStringList lines = jobOutput.split('\n', QString::SkipEmptyParts);

    DVCScommit item;
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

QStringList GitExecutor::getLsFiles(const QString &directory, const QStringList &args)
{
    DVCSjob* job = lsFiles(directory, args);
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

DVCSjob* GitExecutor::gitRevParse(const QString &repository, const QStringList &args)
{
    //Use prepareJob() here only if you like "dead" recursion and KDevelop crashes
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (job)
    {
        QString workDir = repository;
        QFileInfo fsObject(workDir);
        if (fsObject.isFile())
            workDir = fsObject.path();

        job->clear();
        job->setDirectory(workDir);
        *job << "git-rev-parse";
        foreach(const QString &arg, args)
            *job << arg;
        return job;
    }
    if (job) delete job;
    return NULL;
}

DVCSjob* GitExecutor::gitRevList(const QString &repository, const QStringList &args)
{
    DVCSjob* job = new DVCSjob(vcsplugin);
    if (prepareJob(job, repository) ) {
        *job << "git-rev-list";
        foreach(const QString &arg, args)
            *job << arg;
        return job;
    }
    if (job) delete job;
    return NULL;
}

KDevelop::VcsStatusInfo::State GitExecutor::charToState(const char ch)
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

// #include "gitexetor.moc"
