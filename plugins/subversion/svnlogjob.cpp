/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "svnlogjob.h"
#include "svnlogjob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>
#include <kdebug.h>
#include <ThreadWeaver.h>

#include "svnclient.h"

SvnInternalLogJob::SvnInternalLogJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
    m_endRevision.setRevisionValue( qVariantFromValue( KDevelop::VcsRevision::Start ),
                                    KDevelop::VcsRevision::Special );
    m_startRevision.setRevisionValue( qVariantFromValue( KDevelop::VcsRevision::Head ),
                                    KDevelop::VcsRevision::Special );
    m_limit = 0;
}

void SvnInternalLogJob::run()
{
    initBeforeRun();

    SvnClient cli(m_ctxt);
    connect( &cli, SIGNAL(logEventReceived(KDevelop::VcsEvent)),
             this, SIGNAL(logEvent(KDevelop::VcsEvent)) );
    try
    {
        QByteArray ba = location().toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
        cli.log( ba.data(),
                 createSvnCppRevisionFromVcsRevision( startRevision() ),
                 createSvnCppRevisionFromVcsRevision( endRevision() ),
                 limit() );
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while logging file: "
                << location()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalLogJob::setLocation( const KUrl& url )
{
    QMutexLocker l( m_mutex );
    m_location = url;
}

KUrl SvnInternalLogJob::location() const
{
    QMutexLocker l( m_mutex );
    return m_location;
}

KDevelop::VcsRevision SvnInternalLogJob::startRevision() const
{
    QMutexLocker l( m_mutex );
    return m_startRevision;
}

KDevelop::VcsRevision SvnInternalLogJob::endRevision() const
{
    QMutexLocker l( m_mutex );
    return m_endRevision;
}

int SvnInternalLogJob::limit() const
{
    QMutexLocker l( m_mutex );
    return m_limit;
}

void SvnInternalLogJob::setStartRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( m_mutex );
    m_startRevision = rev;
}

void SvnInternalLogJob::setEndRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( m_mutex );
    m_endRevision = rev;
}

void SvnInternalLogJob::setLimit( int limit )
{
    QMutexLocker l( m_mutex );
    m_limit = limit;
}

SvnLogJob::SvnLogJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Log );
    m_job = new SvnInternalLogJob( this );

    setObjectName(i18n("Subversion Log"));
}

QVariant SvnLogJob::fetchResults()
{
    QList<QVariant> list = m_eventList;
    m_eventList.clear();
    return list;
}

void SvnLogJob::start()
{
    if( !m_job->location().isValid() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to log location" ) );
    }else
    {
        connect( m_job, SIGNAL(logEvent(KDevelop::VcsEvent)),
               this, SLOT(logEventReceived(KDevelop::VcsEvent)), Qt::QueuedConnection );
        kDebug(9510) << "logging url:" << m_job->location();
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnLogJob::internalJob() const
{
    return m_job;
}

void SvnLogJob::setLocation( const KUrl& url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocation( url );
}

void SvnLogJob::setStartRevision( const KDevelop::VcsRevision& rev )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setStartRevision( rev );
}

void SvnLogJob::setEndRevision( const KDevelop::VcsRevision& rev )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setEndRevision( rev );
}

void SvnLogJob::setLimit( int limit )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLimit( limit );
}

void SvnLogJob::logEventReceived( const KDevelop::VcsEvent& ev )
{
    m_eventList << qVariantFromValue( ev );
    emit resultsReady( this );
}
