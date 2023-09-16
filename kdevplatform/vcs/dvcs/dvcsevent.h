/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_DVCSEVENT_H
#define KDEVPLATFORM_DVCSEVENT_H

#include <QMetaType>
#include <QSharedDataPointer>

#include <vcs/vcsexport.h>

#include <QStringList>
class QString;

namespace KDevelop
{
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
class KDEVPLATFORMVCS_EXPORT DVcsEvent
{
public:

    /**
     * The CommitType namespace specifies the type of commit. It's mostly used in CommitViewDelegate to
     * choose what to draw in each graph (and for creation of HEADs labels in shortlog).
     * Every commit has a properties array of CommitType (which can be called mask),
     * which shows its state in every branch. for example:
     * if branch has this commit (after merge several branches can have one commit) properties[branch] is set to BRANCH.
     */
    enum CommitType {
        INITIAL, /**< Initial (first) commit, we shouldn't draw bottom line to connect with parent */
        HEAD,    /**< HEAD commit, we should use its for branch label, and shouldn't draw head line to connect with child */
        BRANCH,  /**< draw a circle in the branch column */
        MERGE,   /**< draw a square (current implementation requires drawing connections */
        CROSS,   /**< just draw a cross-line */
        HCROSS,  /**< draw horizontal cross (it's used to connect merge with parent/child */
        MERGE_RIGHT, /**< draw connection lines, two lines (like a clock 9-center-12) */
        MERGE_LEFT,  /**< draw connection lines, 9-center-6) */
        EMPTY        /**< draw nothing */
    };

    DVcsEvent();
    DVcsEvent(const DVcsEvent& rhs);
    ~DVcsEvent();

    DVcsEvent& operator=(const DVcsEvent& rhs);

    /** Returns sha string of the commit. */
    QString commit() const;

    /** Sets sha string. */
    void setCommit(const QString& commit);

    /** Returns QStringList with parents (sha strings). */
    QStringList parents() const;

    /** Sets parents (sha strings). */
    void setParents(const QStringList& parents);

    /** Returns commit date (stored in QString). */
    QString date() const;

    /** Sets date. */
    void setDate(const QString& date);

    /** Returns author (committer) name. */
    QString author() const;

    /** Sets author (committer) name. */
    void setAuthor(const QString& author);

    /** Returns full log in one QString. */
    QString log() const;

    /** Sets full log in one QString. */
    void setLog(const QString& log);

    /** Returns CommitType */
    int type() const;

    /** Sets CommitType */
    void setType(CommitType t);

    /** Returns list of properties */
    QList<int> properties() const;

    /** Sets properties */
    void setProperties(const QList<int>& properties);

    /** Sets property
     * @param index index in the properties array.
     * @param prop value to be set
     */
    void setProperty(int index, int prop);

private:
    QSharedDataPointer<class DVcsEventPrivate> d;
};

}

Q_DECLARE_TYPEINFO(KDevelop::DVcsEvent, Q_MOVABLE_TYPE);

#endif
