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

#ifndef SUBVERSION_UTILS_H
#define SUBVERSION_UTILS_H

#define SVN_LOGVIEW  10
#define SVN_BLAME    11
#define SVN_CHECKOUT 12
#define SVN_ADD      13
#define SVN_DELETE   14
#define SVN_COMMIT   15
#define SVN_UPDATE   16
#define SVN_STATUS   17
#define SVN_INFO     18
#define SVN_DIFF     19
#define SVN_IMPORT   20
#define SVN_REVERT   21
#define SVN_COPY     22
#define SVN_MOVE     23

struct svn_opt_revision_t;

#include <QDateTime>

namespace SvnUtils
{

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

    /// Construct InValid (Unspecified) Revision.
    SvnRevision();

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

private:
    enum RevType
    {
        number = 0,
        kind = 1,
        date = 2
    };
    RevType type;

    long int revNum;
    RevKeyword revKind;
    QDateTime revDate;
};

} // end of namespace SvnRevision

using namespace SvnUtils;

#endif
