/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SUBVERSION_REVISION_H
#define SUBVERSION_REVISION_H

#define SVN_LOGVIEW  (KDevelop::VcsJob::Log)
#define SVN_BLAME    (KDevelop::VcsJob::Annotate)
#define SVN_CHECKOUT (KDevelop::VcsJob::Checkout)
#define SVN_ADD      (KDevelop::VcsJob::Add)
#define SVN_DELETE   (KDevelop::VcsJob::Remove)
#define SVN_COMMIT   (KDevelop::VcsJob::Commit)
#define SVN_UPDATE   (KDevelop::VcsJob::Update)
#define SVN_STATUS   (KDevelop::VcsJob::Status)
#define SVN_DIFF     (KDevelop::VcsJob::Diff)
#define SVN_IMPORT   (KDevelop::VcsJob::Import)
#define SVN_REVERT   (KDevelop::VcsJob::Revert)
#define SVN_COPY     (KDevelop::VcsJob::Copy)
#define SVN_MOVE     (KDevelop::VcsJob::Move)

// subversion internals
#define SVN_INFO     (101)

struct svn_opt_revision_t;

#include <QDateTime>
#include <QVariant>

namespace KDevelop
{
class VcsRevision;
}

class SvnRevision
{
public:
    /** To be used with setRevision( RevKeyword )
     */
    enum RevKeyword
    {
        UNSPECIFIED = 0,
        COMMITTED = 1,
        PREV  = 2,
        BASE = 3,
        WORKING = 4,
        HEAD = 5
    };
    enum RevType
    {
        number = 0,
        kind = 1,
        date = 2
    };

    /// Construct InValid (Unspecified) Revision.
    SvnRevision();

    /// Construct using information from VcsRevision.
    void fromVcsRevision( const KDevelop::VcsRevision &vcsRev );

    /// Specify revision as number.
    void setNumber( long int revnum );

    /** Specify revision as keyword. Supported string is
     *  WORKING, BASE, HEAD, COMMITTED, PREV and UNSPECIFIED
     */
    void setKey( RevKeyword key );

    /// Specify revision as date,time
    void setDate( const QDateTime& date );

    /// Overloaded method. Same with above.
    void setDate( const QDate& aDate, const QTime& aTime = QTime() );

    /// Returns SVN C-Api compatible struct. Use this to call svn_client_* and etc.
    svn_opt_revision_t revision();

    bool isValid();

private:
    RevType type;

    long int revNum;
    RevKeyword revKind;
    QDateTime revDate;
};

// Q_DECLARE_METATYPE(SvnUtils::SvnRevision::RevKeyword)

#endif
