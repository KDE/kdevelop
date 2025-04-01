/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svncopyjob.h"
#include "svncopyjob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"

SvnInternalCopyJob::SvnInternalCopyJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalCopyJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    try
    {
        QByteArray srcba = sourceLocation().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        QByteArray dstba = destinationLocation().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        cli.copy( svn::Path( srcba.data() ), svn::Revision(), svn::Path( dstba.data() ) );
    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while copying file: "
                << sourceLocation() << "to" << destinationLocation()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalCopyJob::setDestinationLocation( const QUrl &url )
{
    QMutexLocker l( &m_mutex );
    m_destinationLocation = url;
}

QUrl SvnInternalCopyJob::destinationLocation() const
{
    QMutexLocker l( &m_mutex );
    return m_destinationLocation;
}

void SvnInternalCopyJob::setSourceLocation( const QUrl &url )
{
    QMutexLocker l( &m_mutex );
    m_sourceLocation = url;
}

QUrl SvnInternalCopyJob::sourceLocation() const
{
    QMutexLocker l( &m_mutex );
    return m_sourceLocation;
}

SvnCopyJob::SvnCopyJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Copy );
    setObjectName(i18n("Subversion Copy"));
}

QVariant SvnCopyJob::fetchResults()
{
    return QVariant();
}

void SvnCopyJob::start()
{
    if ( m_job->sourceLocation().isEmpty() || m_job->destinationLocation().isEmpty() ) {
        failToStart(i18n("Not enough information to copy file"));
    } else {
        qCDebug(PLUGIN_SVN) << "copying url:" << m_job->sourceLocation() << "to url" << m_job->destinationLocation();
        startInternalJob();
    }
}

void SvnCopyJob::setDestinationLocation( const QUrl &url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setDestinationLocation( url );
}

void SvnCopyJob::setSourceLocation( const QUrl &url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setSourceLocation( url );
}

#include "moc_svncopyjob.cpp"
#include "moc_svncopyjob_p.cpp"
