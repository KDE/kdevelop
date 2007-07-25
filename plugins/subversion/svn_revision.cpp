/***************************************************************************
 *   Copyright 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svn_revision.h"
extern "C" {
#include <svn_opt.h>
}
#include "vcsrevision.h"

SvnRevision::SvnRevision()
: type( kind ), revNum(-1), revKind( UNSPECIFIED )
{}

void SvnRevision::fromVcsRevision( const KDevelop::VcsRevision &vcsRev )
{
    KDevelop::VcsRevision::RevisionType vcsRevType = vcsRev.revisionType();
    QString revVal = vcsRev.revisionValue();

    if( vcsRevType == KDevelop::VcsRevision::Special ){

        if( revVal.compare("HEAD", Qt::CaseInsensitive ) == 0 ){
            setKey( HEAD );
        }
        else if( revVal.compare("WORKING", Qt::CaseInsensitive ) == 0 ){
            setKey( WORKING );
        }
        else if( revVal.compare("BASE", Qt::CaseInsensitive ) == 0 ){
            setKey( BASE );
        }
        else if( revVal.compare("PREVIOUS", Qt::CaseInsensitive ) == 0){
            setKey( PREV );
        }
    }
    else if( type == KDevelop::VcsRevision::GlobalNumber ){
        setNumber( revVal.toInt() );
    }
    else if( type == KDevelop::VcsRevision::Date ){
        setDate( QDateTime::fromString( revVal, Qt::ISODate) );
    }
    else{
        // invalid rev.
        type = kind;
        revNum = -1;
        revKind = UNSPECIFIED;
    }
}

void SvnRevision::setNumber( long int revnum )
{
    type = number;
    revNum = revnum;
}

void SvnRevision::setKey( RevKeyword keyword )
{
    type = kind;
    revKind = keyword;
}

void SvnRevision::setDate( const QDateTime& aDate )
{
    type = date;
    revDate = aDate;
}

void SvnRevision::setDate( const QDate& aDate, const QTime& aTime )
{
    setDate( QDateTime( aDate, aTime ) );
}

svn_opt_revision_t SvnRevision::revision()
{
    svn_opt_revision_t ret;
    switch( type ){
        case number:
            ret.value.number = revNum;
            ret.kind = svn_opt_revision_number;
            break;
        case kind:
            ret.value.number = -1;
            if( revKind == WORKING ){
                ret.kind = svn_opt_revision_working;
            } else if ( revKind == BASE ) {
                ret.kind = svn_opt_revision_base;
            } else if ( revKind == HEAD ) {
                ret.kind = svn_opt_revision_head;
            } else if ( revKind == COMMITTED ) {
                ret.kind = svn_opt_revision_committed;
            } else if ( revKind == PREV ) {
                ret.kind = svn_opt_revision_previous;
            } else if ( revKind == UNSPECIFIED ) {
                ret.kind = svn_opt_revision_unspecified;
            } else{
                ret.kind = svn_opt_revision_unspecified;
            }
            break;
        case date:
            ret.kind = svn_opt_revision_date;

            // Note: from apr_time.h. This is used as subversion revision specifier.
            // /* number of microseconds since 00:00:00 january 1, 1970 UTC */
            // typedef apr_int64_t apr_time_t;
            ret.value.date = ((apr_time_t)(revDate.toTime_t())) * 1000000;
            break;
        default:
            ret.kind = svn_opt_revision_unspecified;
            break;
    }

    return ret;
}

bool SvnRevision::isValid()
{
    if( type == SvnRevision::kind ){
        if( revKind == SvnRevision::UNSPECIFIED )
            return false;
    }

    if( type == SvnRevision::number ){
        if( revNum < 0 )
            return false;
    }
    return true;
}
