/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnlogjob.h"
#include "svnlogjob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "svnclient.h"

SvnInternalLogJob::SvnInternalLogJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
    m_endRevision.setRevisionValue(QVariant::fromValue(KDevelop::VcsRevision::Start),
                                    KDevelop::VcsRevision::Special );
    m_startRevision.setRevisionValue(QVariant::fromValue(KDevelop::VcsRevision::Head),
                                    KDevelop::VcsRevision::Special );
    m_limit = 0;
}

void SvnInternalLogJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    SvnClient cli(m_ctxt);
    connect( &cli, &SvnClient::logEventReceived,
             this, &SvnInternalLogJob::logEvent );
    try
    {
        QByteArray ba = location().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        cli.log( ba.data(),
                 createSvnCppRevisionFromVcsRevision( startRevision() ),
                 createSvnCppRevisionFromVcsRevision( endRevision() ),
                 limit() );
    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while logging file: "
                << location()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalLogJob::setLocation( const QUrl &url )
{
    QMutexLocker l( &m_mutex );
    m_location = url;
}

QUrl SvnInternalLogJob::location() const
{
    QMutexLocker l( &m_mutex );
    return m_location;
}

KDevelop::VcsRevision SvnInternalLogJob::startRevision() const
{
    QMutexLocker l( &m_mutex );
    return m_startRevision;
}

KDevelop::VcsRevision SvnInternalLogJob::endRevision() const
{
    QMutexLocker l( &m_mutex );
    return m_endRevision;
}

int SvnInternalLogJob::limit() const
{
    QMutexLocker l( &m_mutex );
    return m_limit;
}

void SvnInternalLogJob::setStartRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( &m_mutex );
    m_startRevision = rev;
}

void SvnInternalLogJob::setEndRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( &m_mutex );
    m_endRevision = rev;
}

void SvnInternalLogJob::setLimit( int limit )
{
    QMutexLocker l( &m_mutex );
    m_limit = limit;
}

SvnLogJob::SvnLogJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Log );
    connect( m_job.data(), &SvnInternalLogJob::logEvent,
             this, &SvnLogJob::logEventReceived );

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
        failToStart(i18n("Not enough information to log location"));
    }else
    {
        qCDebug(PLUGIN_SVN) << "logging url:" << m_job->location();
        startInternalJob();
    }
}

void SvnLogJob::setLocation( const QUrl &url )
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
    m_eventList << QVariant::fromValue(ev);
    emit resultsReady( this );
}

#include "moc_svnlogjob_p.cpp"
#include "moc_svnlogjob.cpp"
