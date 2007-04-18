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
 
#include "subversion_utils.h"
#include <subversion-1/svn_opt.h>

#include <QString>
#include <QDateTime>

namespace SvnUtils
{

SvnRevision::SvnRevision( int aRev, QString aRevKind, QDateTime aRevDate )
: revNum( aRev ), revKind( aRevKind ), revDate( aRevDate )
{}

SvnRevision::SvnRevision()
: revNum(-1), revKind( "UNSPECIFIED" )
{}

svn_opt_revision_t createRevision( long int revision, const QString& revkind )
{
    svn_opt_revision_t result/*,endrev*/;

    if ( revision != -1 ) {
        result.value.number = revision;
        result.kind = svn_opt_revision_number;
    } else if ( revkind == "WORKING" ) {
        result.kind = svn_opt_revision_working;
    } else if ( revkind == "BASE" ) {
        result.kind = svn_opt_revision_base;
    } else if ( revkind == "HEAD" ) {
        result.kind = svn_opt_revision_head;
    } else if ( revkind == "COMMITTED" ) {
        result.kind = svn_opt_revision_committed;
    } else if ( revkind == "PREV" ) {
        result.kind = svn_opt_revision_previous;
    } else if ( revkind == "UNSPECIFIED" ) {
        result.kind = svn_opt_revision_unspecified;
    }
    else {
        result.kind = svn_opt_revision_unspecified;
    }
    return result;
}

svn_opt_revision_t createRevision( SvnRevision &revision )
{
    return createRevision( revision.revNum, revision.revKind );
}

} // end of namespace SvnUtils
