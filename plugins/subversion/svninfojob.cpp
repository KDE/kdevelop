/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svninfojob.h"

#include "svninfojob_p.h"

#include <QMutexLocker>
#include <QTimeZone>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/info.hpp"

SvnInternalInfoJob::SvnInternalInfoJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalInfoJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();
    svn::Client cli(m_ctxt);
    try
    {
        QByteArray ba = location().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        svn::InfoVector v = cli.info( ba.data() );
        svn::Info i = v.at(0);
        SvnInfoHolder h;
        h.name = QString::fromUtf8( i.path().path().c_str() );
        h.url = QUrl::fromUserInput( QString::fromUtf8( i.url() ) );
        h.rev = qlonglong( i.revision() );
        h.kind = i.kind();
        h.repoUrl = QUrl::fromUserInput( QString::fromUtf8( i.repos() ) );
        h.repouuid = QString::fromUtf8( i.uuid() );
        h.lastChangedRev = qlonglong( i.lastChangedRevision() );
        h.lastChangedDate = QDateTime::fromSecsSinceEpoch(i.lastChangedDate(), QTimeZone::LocalTime);
        h.lastChangedAuthor = QString::fromUtf8( i.lastChangedAuthor() );
        h.scheduled = i.schedule();
        h.copyFromUrl = QUrl::fromUserInput( QString::fromUtf8( i.copyFromUrl() ) );
        h.copyFromRevision = qlonglong( i.copyFromRevision() );
        h.textTime = QDateTime::fromSecsSinceEpoch(i.textTime(), QTimeZone::LocalTime);
        h.propertyTime = QDateTime::fromSecsSinceEpoch(i.propertyTime(), QTimeZone::LocalTime);
        h.oldFileConflict = QString::fromUtf8( i.oldConflictFile() );
        h.newFileConflict = QString::fromUtf8( i.newConflictFile() );
        h.workingCopyFileConflict = QString::fromUtf8( i.workingConflictFile() );
        h.propertyRejectFile = QString::fromUtf8( i.propertyRejectFile() );

        emit gotInfo( h );
    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while getting info for file: "
                << m_location
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalInfoJob::setLocation( const QUrl &url )
{
    QMutexLocker l( &m_mutex );
    m_location = url;
}

QUrl SvnInternalInfoJob::location() const
{
    QMutexLocker l( &m_mutex );
    return m_location;
}

SvnInfoJob::SvnInfoJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent ), m_provideInfo( SvnInfoJob::AllInfo )
{
    setType( KDevelop::VcsJob::Add );
    connect( m_job.data(), &SvnInternalInfoJob::gotInfo,
             this, &SvnInfoJob::setInfo, Qt::QueuedConnection );
    setObjectName(i18n("Subversion Info"));
}

QVariant SvnInfoJob::fetchResults()
{
    if( m_provideInfo == RepoUrlOnly )
    {
        return QVariant(m_info.url);
    }else if( m_provideInfo == RevisionOnly )
    {
        KDevelop::VcsRevision rev;
        svn::Revision svnRev( m_info.rev );
        switch( m_provideRevisionType )
        {
            case KDevelop::VcsRevision::Date:
                rev.setRevisionValue(QVariant(QDateTime::fromSecsSinceEpoch(svnRev.date(), QTimeZone::LocalTime)),
                                     KDevelop::VcsRevision::Date);
                break;
            default:
                rev.setRevisionValue( QVariant( qlonglong( svnRev.revnum() ) ),
                                      KDevelop::VcsRevision::GlobalNumber );
                break;
        }
        return QVariant::fromValue<KDevelop::VcsRevision>(rev);
    }
    return QVariant::fromValue<SvnInfoHolder>(m_info);
}

void SvnInfoJob::start()
{
    if (!m_job->location().isValid()) {
        failToStart(i18n("Not enough information to execute info job"));
    } else {
        startInternalJob();
    }
}

void SvnInfoJob::setLocation( const QUrl &url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocation( url );
}

void SvnInfoJob::setProvideInformation( ProvideInformationType type )
{
    m_provideInfo = type;
}

void SvnInfoJob::setProvideRevisionType( KDevelop::VcsRevision::RevisionType t )
{
    m_provideRevisionType = t;
}

void SvnInfoJob::setInfo( const SvnInfoHolder& info )
{
    m_info = info;
    emit resultsReady( this );
}

#include "moc_svninfojob.cpp"
#include "moc_svninfojob_p.cpp"
