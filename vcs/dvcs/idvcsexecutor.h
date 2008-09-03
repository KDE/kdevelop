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

#ifndef IDVCS_EXECUTOR_H
#define IDVCS_EXECUTOR_H

#include<QtCore/QStringList>
#include<QtCore/QList>
#include <KDE/KUrl>

#include "../vcsstatusinfo.h"

class QVariant;
class KUrl;
class DVCSjob;

/**
 * This class is used to store all required Commit(Revision) data: commit (sha string),
 * log (log[0] is used as shortlog), author, date (in QString), CommitType, and a special properties mask.
 * It's mostly used in CommitViewDelegate. Maybe this class can be merged with
 * something like KDevelop::VcsEvent.
 * This is also a helper class for the LogView::parseOutput() method.
 *
 * @note DVCScommit is used just to store the data, it doesn't change any of it (for example with QString::trimmed())
 *
 * @see GitExecutor::getAllCommits()
 * @see end of CommitViewDelegate::paintGraph() for details of how properties are used
 * @see dvcsEcecutor::parseOutput()
 *
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 *
 */
//TODO: properties should be used instead of type
class DVCScommit {
public:

    /**
     * The CommitType namespace specifies the type of commit. It's mostly used in CommitViewDelegate to
     * choose what to draw in each graph (and for creation of HEADs labels in shortlog).
     * Every commit has a properties array of CommitType (which can be called mask), 
     * which shows its state in every branch. for example:
     * if branch has this commit (after merge several branches can have one commit) properties[branch] is set to BRANCH.
     */
    enum CommitType {
        INITIAL, /**< Initial (first) commit, we should'n draw bottom line to connect with parent */
        HEAD,    /**< HEAD commit, we should use its for branch label, and shouldn't draw head line to connect with child */
        BRANCH,  /**< draw a circle in the branch column */
        MERGE,   /**< draw a square (current implementation requires drawing connections */
        CROSS,   /**< just draw a cross-line */
        HCROSS,  /**< draw horizontal cross (it's used to connect merge with parent/child */
        MERGE_RIGHT, /**< draw connection lines, two lines (like a clock 9-center-12) */
        MERGE_LEFT,  /**< draw connection lines, 9-center-6) */
        EMPTY        /**< draw nothing */
    };

    /** Returns sha string of the commit. */
    QString getCommit() const {return commit;}

    /** Sets sha string. */
    void setCommit(const QString &_commit_) 
    {
        commit = _commit_;
    }

    /** Returns QStringList with parents (sha strings). */
    QStringList getParents() const {return parents;}

    /** Sets parents (sha strings). */
    void setParents(const QStringList &_parents_) 
    {
        parents = _parents_;
        switch (parents.count())
        {
        case 0:
        {
            setType(INITIAL);
            break;
        }
        case 1:
        {
            setType(BRANCH);
            break;
        }
        default: // > 1
        {
            setType(MERGE);
            break;
        }
        }
    }

    /** Returns commit date (stored in QString). */
    QString getDate() const {return date;}

    /** Sets date. */
    void setDate(const QString &_date_) {date = _date_;}

    /** Returns author (committer) name. */
    QString getAuthor() const {return author;}

    /** Sets author (committer) name. */
    void setAuthor(const QString &_author_) {author = _author_;}

    /** Returns full log in one QString. */
    QString getLog() const {return log;}

    /** Sets full log in one QString. */
    void setLog(const QString &_log_) {log = _log_;}

    /** Returns CommitType */
    int getType() const {return type;}

    /** Sets CommitType */
    void setType(CommitType t) {type = t;}

    /** Returns list of properties */
    QList<int>getProperties() const {return properties;}

    /** Sets properties */
    void setProperties(const QList<int> &prop) {properties = prop;}

    /** Sets property
     * @param index index in the properties array.
     * @param prop value to be set
     */
    void setPropetry(const int index, const int prop) {if (index >= 0 && index < properties.count())properties[index] = prop;}
private:
    int type;
private:
    QString commit;
    QStringList parents;
    QString date;
    QString author;
    QString log;
    QList<int>properties; //used to describe graph columns in every row (MERGE, HEAD, CROSS, etc)
};

namespace KDevelop
{

/**
 * This proxy acts as a single point of entry for most of the common dvcs commands.
 * It is very easy to use, as the caller does not have to deal which the DVCSjob class directly.
 * All the command line generation and job handling is done internally. The caller gets a DVCSjob
 * object returned from the proxy and can then call it's start() method.
 *
 * Here is and example of how to use the proxy:
 * @code
 * DVCSjob* job = proxy->add(repo, urls);
 * if ( job ) {
 *     connect(job, SIGNAL(result(KJob*)),
 *             this, SIGNAL(jobFinished(KJob*)));
 *     job->start();
 * }
 * @endcode
 *
 * @note All actions that take a KUrl::List also need an url to the repository which
 *       must be a common base directory to all files from the KUrl::List.
 *       Actions that just take a single KUrl don't need a repository, the dvcs command will be
 *       called directly in the directory of the given file
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class KDEVPLATFORMVCS_EXPORT IDVCSexecutor
{
public:
    virtual ~IDVCSexecutor() {}

    /** Checks if dirPath is located in DVCS repository */
    virtual bool isValidDirectory(const KUrl &dirPath) = 0;

    /** This method checks if the path is already in the repo. If the path is directory it returns
     * isValidDirectory(), else it returns if the file has been already added to the repo.
     * @see isValidDirectory(const KUrl &dirPath). */
    virtual bool isInRepo(const KUrl &path);

    /** Returns DVCS name, I hope it returs "git" for you */
    virtual QString name() const = 0;

    /** Call this method to init repository in directory */
    virtual DVCSjob* init(const KUrl & directory) = 0;

    /** Clones repository
     * @param directory is a destination folder
     * @param repository is a repo to clone
     */
    virtual DVCSjob* clone(const KUrl &directory, const KUrl repository) = 0;

    /** Add files to the index. In KDevelop files always contains one item (it's called with an context menu) */
    virtual DVCSjob* add(const QString& repository, const KUrl::List &files) = 0;

    /** This method does a commit */
    virtual DVCSjob* commit(const QString& repository,
                            const QString& message = "KDevelop did not provide any message, it may be a bug",
                            const KUrl::List& args = QStringList("-a")) = 0;
    /** Remove files from a repository */
    virtual DVCSjob* remove(const QString& repository, const KUrl::List& files) = 0;

    /** Status */
    virtual DVCSjob* status(const QString & repo, const KUrl::List & files,
                            bool recursive=false, bool taginfo=false) = 0;

    //TODO: implement in Hg and Bazaar then make pure virtual
    /** Log */
    virtual DVCSjob* log(const KUrl& url);

    /*    virtual DVCSjob* is_inside_work_tree(const QString& repository) = 0;*/
    /** empty_cmd is used when something is not implemented, but has to return any job */
    virtual DVCSjob* empty_cmd() const = 0;

   /**
     * Parses the output generated by a @code dvcs log @endcode command and
     * fills the given QList with all commits infos found in the given output.
     * @param jobOutput Pass in the plain output of a @code dvcs log @endcode job
     * @param revisions Will be filled with all revision infos found in @p jobOutput
     */
    ///Todo: require renaming to parseLogOutput
    virtual void parseOutput(const QString& jobOutput,
                             QList<DVCScommit>& revisions) const;

    /** Checkout. */
    virtual DVCSjob* checkout(const QString &repository, const QString &branch);

    /** Branch. */
    virtual DVCSjob* branch(const QString &repository, const QString &basebranch = QString(), 
                            const QString &branch = QString(), const QStringList &args = QStringList());
    //parsers for branch:
    /** Returns current branch. */
    virtual QString curBranch(const QString &repository);

    /** Returns the list of branches. */
    virtual QStringList branches(const QString &repository);

    //commit dialog helpers:
    /** Returns the list of modified files (diff between workdir and index). */
    virtual QList<QVariant> getModifiedFiles(const QString &directory);
    /** Returns the list of already cached files (diff between index and HEAD). */
    virtual QList<QVariant> getCachedFiles(const QString &directory);
    /** Files are not in the repo, but in the repository location. */
    virtual QList<QVariant> getOtherFiles(const QString &directory);

    /** Reset */
    virtual DVCSjob* reset(const QString &repository, const QStringList &args, const KUrl::List &files);

    /** Returs the list of all commits (in all branches).
     * @see CommitView and CommitViewDelegate to see how this list is used.
     */
    virtual QList<DVCScommit> getAllCommits(const QString &repo);


protected:
    /** RequestedOperation is used to know if we should check the repo with isValidDirectory 
     * or we want to create new repo (init/clone).
     */
    enum RequestedOperation {
        NormalOperation, /**< add/commit/etc, check if we are in the repo */
        Init             /**< we need init/clone, so don't call isValidDirectory, we're not in the repo, but yet ;) */
    };

    /** This method checks RequestedOperation, clears the job and sets working directory.
     * Returns false only if op == NormalOperation and we are not in the repository.
     * @param job the DVCSjob to be prepared
     * @param repository working directiry
     * @param op shows if the method should run isValidDirectory
     */
    virtual bool prepareJob(DVCSjob* job, const QString& repository, 
                            enum RequestedOperation op = NormalOperation);
    /** Add files as args to the job. It changes absolute pathes to relatives */
    static bool addFileList(DVCSjob* job, const KUrl::List& urls);

};

}

#endif
