/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnmovejob.h"
#include "svnmovejob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"

SvnInternalMoveJob::SvnInternalMoveJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalMoveJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    try
    {
        QByteArray srcba = sourceLocation().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        QByteArray dstba = destinationLocation().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        cli.move( svn::Path( srcba.data() ), svn::Revision(), svn::Path( dstba.data() ), force() );
    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while moving file: "
                << sourceLocation() << "to" << destinationLocation()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalMoveJob::setDestinationLocation( const QUrl &url )
{
    QMutexLocker l( &m_mutex );
    m_destinationLocation = url;
}

QUrl SvnInternalMoveJob::destinationLocation() const
{
    QMutexLocker l( &m_mutex );
    return m_destinationLocation;
}

void SvnInternalMoveJob::setSourceLocation( const QUrl &url )
{
    QMutexLocker l( &m_mutex );
    m_sourceLocation = url;
}

QUrl SvnInternalMoveJob::sourceLocation() const
{
    QMutexLocker l( &m_mutex );
    return m_sourceLocation;
}

bool SvnInternalMoveJob::force() const
{
    QMutexLocker l( &m_mutex );
    return m_force;
}

void SvnInternalMoveJob::setForce( bool force )
{
    QMutexLocker l( &m_mutex );
    m_force = force;
}

SvnMoveJob::SvnMoveJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Move );
    setObjectName(i18n("Subversion Move"));
}

QVariant SvnMoveJob::fetchResults()
{
    return QVariant();
}

void SvnMoveJob::start()
{
    if ( m_job->sourceLocation().isEmpty() || m_job->destinationLocation().isEmpty() ) {
        failToStart(i18n("Not enough information to move file"));
    } else {
        qCDebug(PLUGIN_SVN) << "moving url:" << m_job->sourceLocation() << "to url" << m_job->destinationLocation();
        startInternalJob();
    }
}

void SvnMoveJob::setDestinationLocation( const QUrl &url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setDestinationLocation( url );
}

void SvnMoveJob::setSourceLocation( const QUrl &url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setSourceLocation( url );
}

void SvnMoveJob::setForce( bool force )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setForce( force );
}

#include "moc_svnmovejob_p.cpp"
#include "moc_svnmovejob.cpp"
