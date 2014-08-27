/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Moveright 2007 Andreas Pakulat <apaku@gmx.de>                         *
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
 *   You should have received a move of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "svnmovejob.h"
#include "svnmovejob_p.h"

#include <QMutexLocker>

#include <klocale.h>
#include <kdebug.h>
#include <ThreadWeaver.h>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"

SvnInternalMoveJob::SvnInternalMoveJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent ), m_force( false )
{
}

void SvnInternalMoveJob::run()
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    try
    {
        QByteArray srcba = sourceLocation().toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
        QByteArray dstba = destinationLocation().toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
        cli.move( svn::Path( srcba.data() ), svn::Revision(), svn::Path( dstba.data() ), force() );
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while moveing file: "
                << sourceLocation() << "to" << destinationLocation()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalMoveJob::setDestinationLocation( const KUrl& url )
{
    QMutexLocker l( m_mutex );
    m_destinationLocation = url;
}

KUrl SvnInternalMoveJob::destinationLocation() const
{
    QMutexLocker l( m_mutex );
    return m_destinationLocation;
}

void SvnInternalMoveJob::setSourceLocation( const KUrl& url )
{
    QMutexLocker l( m_mutex );
    m_sourceLocation = url;
}

KUrl SvnInternalMoveJob::sourceLocation() const
{
    QMutexLocker l( m_mutex );
    return m_sourceLocation;
}

bool SvnInternalMoveJob::force() const
{
    QMutexLocker l( m_mutex );
    return m_force;
}

void SvnInternalMoveJob::setForce( bool force )
{
    QMutexLocker l( m_mutex );
    m_force = force;
}

SvnMoveJob::SvnMoveJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Move );
    m_job = new SvnInternalMoveJob( this );
    setObjectName(i18n("Subversion Move"));
}

QVariant SvnMoveJob::fetchResults()
{
    return QVariant();
}

void SvnMoveJob::start()
{
    if( m_job->sourceLocation().isEmpty() || m_job->destinationLocation().isEmpty() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to move file" ) );
    }else
    {
        kDebug(9510) << "moveing url:" << m_job->sourceLocation() << "to url" << m_job->destinationLocation();
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnMoveJob::internalJob() const
{
    return m_job;
}

void SvnMoveJob::setDestinationLocation( const KUrl& url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setDestinationLocation( url );
}

void SvnMoveJob::setSourceLocation( const KUrl& url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setSourceLocation( url );
}

void SvnMoveJob::setForce( bool force )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setForce( force );
}
