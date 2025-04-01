/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnstatusjob.h"
#include "svnstatusjob_p.h"

#include <QMutexLocker>
#include <QUrl>

#include <KLocalizedString>

extern "C"
{
#include <svn_wc.h>
}

#include <iostream>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/status.hpp"

KDevelop::VcsStatusInfo::State getState( const svn::Status& st )
{
    KDevelop::VcsStatusInfo::State state;
    if( st.isVersioned() )
    {
        if( st.textStatus() == svn_wc_status_added )
        {
            state = KDevelop::VcsStatusInfo::ItemAdded;
        }else if( st.textStatus() == svn_wc_status_modified
                    || st.propStatus() == svn_wc_status_modified )
        {
            state = KDevelop::VcsStatusInfo::ItemModified;
        }else if( st.textStatus() == svn_wc_status_deleted )
        {
            state = KDevelop::VcsStatusInfo::ItemDeleted;
        }else if( st.textStatus() == svn_wc_status_conflicted
                    || st.propStatus() == svn_wc_status_conflicted )
        {
            state = KDevelop::VcsStatusInfo::ItemHasConflicts;
        }else
        {
            state = KDevelop::VcsStatusInfo::ItemUpToDate;
        }
    }else
    {
        state = KDevelop::VcsStatusInfo::ItemUnknown;
    }
    return state;
}

SvnInternalStatusJob::SvnInternalStatusJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalStatusJob::setRecursive( bool recursive )
{
    QMutexLocker l( &m_mutex );
    m_recursive = recursive;
}

void SvnInternalStatusJob::setLocations( const QList<QUrl>& urls )
{
    QMutexLocker l( &m_mutex );
    m_locations = urls;
}

QList<QUrl> SvnInternalStatusJob::locations() const
{
    QMutexLocker l( &m_mutex );
    return m_locations;
}
bool SvnInternalStatusJob::recursive() const
{
    QMutexLocker l( &m_mutex );
    return m_recursive;
}

void SvnInternalStatusJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    qCDebug(PLUGIN_SVN) << "Running internal status job with urls:" << m_locations;
    initBeforeRun();

    svn::Client cli(m_ctxt);
    const QList<QUrl> l = locations();
    for (const QUrl& url : l) {
        //qCDebug(PLUGIN_SVN) << "Fetching status info for:" << url;
        try
        {
            QByteArray ba = url.toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
            const svn::StatusEntries se = cli.status(ba.data(), recursive());
            for (auto& statusEntry : se) {
                KDevelop::VcsStatusInfo info;
                info.setUrl(QUrl::fromLocalFile(QString::fromUtf8(statusEntry.path())));
                info.setState(getState(statusEntry));
                emit gotNewStatus( info );
            }
        }catch( const svn::ClientException& ce )
        {
            qCDebug(PLUGIN_SVN) << "Couldn't get status: " << url << QString::fromUtf8( ce.message() );
            setErrorMessage( QString::fromUtf8( ce.message() ) );
            m_success = false;
        }
    }
}

SvnStatusJob::SvnStatusJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Status );
    connect(m_job.data(), &SvnInternalStatusJob::gotNewStatus,
            this, &SvnStatusJob::addToStats, Qt::QueuedConnection);
    setObjectName(i18n("Subversion Status"));
}

QVariant SvnStatusJob::fetchResults()
{
    QList<QVariant> temp = m_stats;
    m_stats.clear();
    return QVariant(temp);
}

void SvnStatusJob::start()
{
    if( m_job->locations().isEmpty() ) {
        failToStart(i18n("Not enough information to execute status job"));
    } else {
        qCDebug(PLUGIN_SVN) << "Starting status job";
        startInternalJob();
    }
}

void SvnStatusJob::setLocations( const QList<QUrl>& urls )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocations( urls );
}

void SvnStatusJob::setRecursive( bool recursive )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setRecursive( recursive );
}

void SvnStatusJob::addToStats( const KDevelop::VcsStatusInfo& info )
{
    //qCDebug(PLUGIN_SVN) << "new status info:" << info.url() << info.state();
    if (!m_stats.contains(QVariant::fromValue(info))) {
        m_stats << QVariant::fromValue(info);
        emit resultsReady( this );
    }else
    {
        qCDebug(PLUGIN_SVN) << "Already have this info:";
    }
}

#include "moc_svnstatusjob_p.cpp"
#include "moc_svnstatusjob.cpp"
