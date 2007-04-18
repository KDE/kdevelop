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

struct svn_opt_revision_t;
#include <QString>
#include <QDateTime>

namespace SvnUtils
{

class SvnRevision
{
public:
    /// Construct Valid Revision
    SvnRevision( int aRev, QString aRevKind, QDateTime aRevDate );
    /// Construct InValid (Unspecified) Revision.
    SvnRevision();
    int revNum;
    QString revKind;
    QDateTime revDate;
};

// TODO support datetime
svn_opt_revision_t createRevision( long int revision, const QString& revkind );
svn_opt_revision_t createRevision( SvnRevision &revision );

} // end of namespace SvnRevision

using namespace SvnUtils;

#endif
