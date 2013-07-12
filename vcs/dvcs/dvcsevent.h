/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_DVCSEVENT_H
#define KDEVPLATFORM_DVCSEVENT_H

#include <QtCore/QStringList>

/**
 * This class is used to store all required Commit(Revision) data: commit (sha string),
 * log (log[0] is used as shortlog), author, date (in QString), CommitType, and a special properties mask.
 * It's mostly used in CommitViewDelegate. Maybe this class can be merged with
 * something like KDevelop::VcsEvent.
 * This is also a helper class for the LogView::parseOutput() method.
 *
 * @note DVcsEvent is used just to store the data, it doesn't change any of it (for example with QString::trimmed())
 *
 * @see GitExecutor::getAllCommits()
 * @see end of CommitViewDelegate::paintGraph() for details of how properties are used
 * @see dvcsEcecutor::parseOutput()
 *
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 *
 */
//TODO: properties should be used instead of type
class DVcsEvent {
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

#endif
