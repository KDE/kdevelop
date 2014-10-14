/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   Parts of the file are copied from the RapidSvn C++ library            *
 *   Copyright (c) 2002-2006 The RapidSvn Group.  All rights reserved.     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "svnclient.h"

#include <QDateTime>
#include <QList>


extern "C" {
#include <svn_client.h>
#include <svn_io.h>
}
#include "kdevsvncpp/targets.hpp"
#include "kdevsvncpp/pool.hpp"
#include <QStandardPaths>

#include <vcs/vcsrevision.h>
#include <vcs/vcsannotation.h>

void fail (apr_pool_t *pool, apr_status_t status, const char *fmt, ...)
{
    va_list ap;
    char *msg;
    svn_error_t * error;

    va_start (ap, fmt);
    msg = apr_pvsprintf (pool, fmt, ap);
    va_end (ap);

    error = svn_error_create (status, NULL, msg);
    throw svn::ClientException (error);
}

void cleanup( apr_file_t* outfile, const char* outfileName, apr_file_t* errfile, const char* errfileName, const svn::Pool& pool )
{
    if( outfile != 0 )
    {
        apr_file_close( outfile );
    }

    if( errfile != 0 )
    {
        apr_file_close( outfile );
    }

    if( outfileName != 0 )
    {
        svn_error_clear( svn_io_remove_file ( outfileName, pool ) );
    }


    if( errfileName != 0 )
    {
        svn_error_clear( svn_io_remove_file ( errfileName, pool ) );
    }

}

SvnClient::SvnClient( svn::Context* ctx )
    : QObject(0), svn::Client( ctx ), m_ctxt( ctx )
{
}

QString SvnClient::diff( const svn::Path& src, const svn::Revision& srcRev,
                const svn::Path& dst, const svn::Revision& dstRev,
                const bool recurse, const bool ignoreAncestry,
                const bool noDiffDeleted, const bool ignoreContentType )
        throw (svn::ClientException)
{
    svn::Pool pool;
    // null options
    apr_array_header_t *options = svn_cstring_split( "", "\t\r\n", false, pool );

    svn_error_t* error;

    const char* outfileName = 0;
    apr_file_t* outfile = 0;
    const char* errfileName = 0;
    apr_file_t* errfile = 0;

    QByteArray ba = QString(QStandardPaths::writableLocation(QStandardItem::TempLocation)+"/kdevelop_svn_diff" ).toUtf8();
    
    error = svn_io_open_unique_file( &outfile, &outfileName, ba.data(), ".tmp", false, pool );

    if( error != 0 )
    {
        ::cleanup( outfile, outfileName, errfile, errfileName, pool );
        throw svn::ClientException( error );
    }

    error = svn_io_open_unique_file( &errfile, &errfileName, ba.data(), ".tmp", false, pool );

    if( error != 0 )
    {
        ::cleanup( outfile, outfileName, errfile, errfileName, pool );
        throw svn::ClientException( error );
    }
        
    error = svn_client_diff3( options,
                            src.c_str(), srcRev.revision(),
                            dst.c_str(), dstRev.revision(),
                            recurse, ignoreAncestry, noDiffDeleted,
                            ignoreContentType, "UTF-8",
                            outfile, errfile, m_ctxt->ctx(), pool );
    if ( error )
    {
        ::cleanup( outfile, outfileName, errfile, errfileName, pool );
        throw svn::ClientException(error);
    }

    // then we reopen outfile for reading
    apr_status_t aprstatus = apr_file_close (outfile);
    if (aprstatus)
    {
      ::cleanup (outfile, outfileName, errfile, errfileName, pool);
      ::fail (pool, aprstatus, "failed to close '%s'", outfileName);
    }

    aprstatus = apr_file_open (&outfile, outfileName, APR_READ, APR_OS_DEFAULT, pool);
    if (aprstatus)
    {
      ::cleanup (outfile, outfileName, errfile, errfileName, pool);
      ::fail (pool, aprstatus, "failed to open '%s'", outfileName);
    }


    svn_stringbuf_t* stringbuf;
    // now we can read the diff output from outfile and return that
    error = svn_stringbuf_from_aprfile (&stringbuf, outfile, pool);

    if (error != NULL)
    {
      ::cleanup (outfile, outfileName, errfile, errfileName, pool);
      throw svn::ClientException (error);
    }

    ::cleanup (outfile, outfileName, errfile, errfileName, pool);
    return QString::fromUtf8( stringbuf->data );
}

QString SvnClient::diff( const svn::Path& src, const svn::Revision& pegRev,
                const svn::Revision& srcRev, const svn::Revision& dstRev,
                const bool recurse, const bool ignoreAncestry,
                const bool noDiffDeleted, const bool ignoreContentType )
        throw (svn::ClientException)
{
    svn::Pool pool;
    // null options
    apr_array_header_t *options = svn_cstring_split( "", "\t\r\n", false, pool );


    svn_error_t* error;

    const char* outfileName = 0;
    apr_file_t* outfile = 0;
    const char* errfileName = 0;
    apr_file_t* errfile = 0;

    QByteArray ba = QStandardPaths::writableLocation("tmp").toUtf8();

    error = svn_io_open_unique_file( &outfile, &outfileName, ba.data(), ".tmp", false, pool );

    if( error != 0 )
    {
        ::cleanup( outfile, outfileName, errfile, errfileName, pool );
        throw svn::ClientException( error );
    }

    error = svn_io_open_unique_file( &errfile, &errfileName, ba.data(), ".tmp", false, pool );

    if( error != 0 )
    {
        ::cleanup( outfile, outfileName, errfile, errfileName, pool );
        throw svn::ClientException( error );
    }

    error = svn_client_diff_peg3( options,
                            src.c_str(), pegRev.revision(),
                            srcRev.revision(), dstRev.revision(),
                            recurse, ignoreAncestry, noDiffDeleted,
                            ignoreContentType, "UTF-8",
                            outfile, errfile, m_ctxt->ctx(), pool );
    if ( error )
    {
        ::cleanup( outfile, outfileName, errfile, errfileName, pool );
        throw svn::ClientException(error);
    }

    // then we reopen outfile for reading
    apr_status_t aprstatus = apr_file_close (outfile);
    if (aprstatus)
    {
      ::cleanup (outfile, outfileName, errfile, errfileName, pool);
      ::fail (pool, aprstatus, "failed to close '%s'", outfileName);
    }

    aprstatus = apr_file_open (&outfile, outfileName, APR_READ, APR_OS_DEFAULT, pool);
    if (aprstatus)
    {
      ::cleanup (outfile, outfileName, errfile, errfileName, pool);
      ::fail (pool, aprstatus, "failed to open '%s'", outfileName);
    }


    svn_stringbuf_t* stringbuf;
    // now we can read the diff output from outfile and return that
    error = svn_stringbuf_from_aprfile (&stringbuf, outfile, pool);

    if (error != NULL)
    {
      ::cleanup (outfile, outfileName, errfile, errfileName, pool);
      throw svn::ClientException(error);
    }

    ::cleanup (outfile, outfileName, errfile, errfileName, pool);
    return QString::fromUtf8( stringbuf->data );
}

static svn_error_t *
kdev_logReceiver (void *baton,
                apr_hash_t * changedPaths,
                svn_revnum_t rev,
                const char *author,
                const char *date,
                const char *msg,
                apr_pool_t * pool)
{
    SvnClient* client = (SvnClient *) baton;

    KDevelop::VcsEvent ev;
    ev.setAuthor( QString::fromUtf8( author ) );
    ev.setDate( QDateTime::fromString( QString::fromUtf8( date ), Qt::ISODate ) );
    ev.setMessage( QString::fromUtf8( msg ) );
    KDevelop::VcsRevision vcsrev;
    vcsrev.setRevisionValue( QVariant( qlonglong( rev ) ), KDevelop::VcsRevision::GlobalNumber );
    ev.setRevision( vcsrev );

    if (changedPaths != NULL)
    {
        for (apr_hash_index_t *hi = apr_hash_first (pool, changedPaths);
            hi != NULL;
            hi = apr_hash_next (hi))
        {
            char *path;
            void *val;
            apr_hash_this (hi, (const void **)&path, NULL, &val);

            svn_log_changed_path_t *log_item = reinterpret_cast<svn_log_changed_path_t *> (val);
            KDevelop::VcsItemEvent iev;
            iev.setRepositoryLocation( QString::fromUtf8( path ) );
            iev.setRepositoryCopySourceLocation( QString::fromUtf8( log_item->copyfrom_path ) );
            KDevelop::VcsRevision irev;
            irev.setRevisionValue( QVariant( qlonglong( log_item->copyfrom_rev ) ),
                                   KDevelop::VcsRevision::GlobalNumber );
            iev.setRepositoryCopySourceRevision( irev );
            switch( log_item->action )
            {
                case 'A':
                    iev.setActions( KDevelop::VcsItemEvent::Added );
                    break;
                case 'M':
                    iev.setActions( KDevelop::VcsItemEvent::Modified  );
                    break;
                case 'D':
                    iev.setActions( KDevelop::VcsItemEvent::Deleted );
                    break;
                case 'R':
                    iev.setActions( KDevelop::VcsItemEvent::Replaced );
                    break;
            }
            
            ev.items().append( iev );
        }
    }
    client->emitLogEventReceived( ev );

    return NULL;
}

void SvnClient::log( const char* path,
                     const svn::Revision& start,
                     const svn::Revision& end,
                     int limit,
                     bool discoverChangedPaths,
                     bool strictNodeHistory )
        throw (svn::ClientException)
{
    svn::Pool pool;
    svn::Targets target(path);
    svn_error_t *error;

    error = svn_client_log2 (
    target.array(pool),
    start.revision(),
    end.revision(),
    limit,
    discoverChangedPaths ? 1 : 0,
    strictNodeHistory ? 1 : 0,
    kdev_logReceiver,
    this,
    m_ctxt->ctx(), // client ctx
    pool);

    if (error != NULL)
    {
        throw svn::ClientException (error);
    }
}

void SvnClient::emitLogEventReceived( const KDevelop::VcsEvent& ev )
{
    emit logEventReceived( ev );
}
