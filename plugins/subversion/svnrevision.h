/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_REVISION_H
#define SVN_REVISION_H

struct svn_opt_revision_t;

#include <QDateTime>
#include <QVariant>

#include <vcsrevision.h>

class SvnRevision
{
public:
    /** To be used with setRevision( Keyword )
     */
    enum Keyword
    {
        Unspecified = 0,
        Committed = 1,
        Prev  = 2,
        Base = 3,
        Working = 4,
        Head = 5
    };
    enum Type
    {
	Number = KDevelop::VcsRevision::GlobalNumber,
	Date = KDevelop::VcsRevision::Date,
        Kind = KDevelop::VcsRevision::UserType+1
    };

    /// Construct InValid (Unspecified) Revision.
    SvnRevision();

    /// Construct using information from VcsRevision.
    void fromVcsRevision( const KDevelop::VcsRevision &vcsRev );

    /// Specify revision as number.
    void setNumber( long int revnum );

    /** Specify revision as keyword. Supported string is
     *  Working, Base, Head, Committed, Prev and Unspecified
     */
    void setKey( SvnRevision::Keyword key );

    /// Specify revision as date,time
    void setDate( const QDateTime& date );

    /// Overloaded method. Same with above.
    void setDate( const QDate& aDate, const QTime& aTime = QTime() );

    /// Returns SVN C-Api compatible struct. Use this to call svn_client_* and etc.
    svn_opt_revision_t revision();

    bool isValid();

private:
    SvnRevision::Type type;

    long int revNum;
    SvnRevision::Keyword revKind;
    QDateTime revDate;
};

// Q_DECLARE_METATYPE(SvnUtils::SvnRevision::Keyword)

#endif
