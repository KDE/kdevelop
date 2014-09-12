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

#include "svnstatusjob.h"
#include "svnstatusjob_p.h"

#include <QFileInfo>
#include <QMutexLocker>

#include <KLocalizedString>
#include <kurl.h>
#include <kdebug.h>
#include <ThreadWeaver.h>

extern "C"
{
#include <svn_wc.h>
}

#include <iostream>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/status.hpp"

KDevelop::VcsStatusInfo::State getState( svn::Status st )
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
    , m_recursive( false )
{
}

void SvnInternalStatusJob::setRecursive( bool recursive )
{
    QMutexLocker l( m_mutex );
    m_recursive = recursive;
}

void SvnInternalStatusJob::setLocations( const KUrl::List& urls )
{
    QMutexLocker l( m_mutex );
    m_locations = urls;
}

KUrl::List SvnInternalStatusJob::locations() const
{
    QMutexLocker l( m_mutex );
    return m_locations;
}
bool SvnInternalStatusJob::recursive() const
{
    QMutexLocker l( m_mutex );
    return m_recursive;
}

void SvnInternalStatusJob::run()
{
    kDebug(9510) << "Running internal status job with urls:" << m_locations;
    initBeforeRun();

    svn::Client cli(m_ctxt);
    KUrl::List l = locations();
    foreach( const KUrl &url, l )
    {
        //kDebug(9510) << "Fetching status info for:" << url;
        try
        {
            QByteArray ba = url.toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
            svn::StatusEntries se = cli.status( ba.data(), recursive() );
            for( svn::StatusEntries::const_iterator it = se.begin(); it != se.end() ; ++it )
            {
                KDevelop::VcsStatusInfo info;
                info.setUrl( KUrl( (*it).path() ) );
                info.setState( getState( *it ) );
                emit gotNewStatus( info );
            }
        }catch( svn::ClientException ce )
        {
            kDebug(9510) << "Couldn't get status: " << url << QString::fromUtf8( ce.message() );
            setErrorMessage( QString::fromUtf8( ce.message() ) );
            m_success = false;
        }
    }
}

SvnStatusJob::SvnStatusJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Status );
    m_job = new SvnInternalStatusJob( this );
    setObjectName(i18n("Subversion Status"));
}

QVariant SvnStatusJob::fetchResults()
{
    QList<QVariant> temp = m_stats;
    m_stats.clear();
    return QVariant(temp);
}

SvnInternalJobBase* SvnStatusJob::internalJob() const
{
    return m_job;
}

void SvnStatusJob::start()
{
    if( m_job->locations().isEmpty() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to execute status job" ) );
    }else
    {
        kDebug(9510) << "Starting status job";
        connect( m_job, SIGNAL(gotNewStatus(KDevelop::VcsStatusInfo)),
            this, SLOT(addToStats(KDevelop::VcsStatusInfo)),
                        Qt::QueuedConnection );
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

void SvnStatusJob::setLocations( const KUrl::List& urls )
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
    //kDebug(9510) << "new status info:" << info.url() << info.state();
    if( !m_stats.contains( qVariantFromValue( info ) ) )
    {
        m_stats << qVariantFromValue( info );
        emit resultsReady( this );
    }else
    {
        kDebug(9510) << "Already have this info:";
    }
}

#include "svnstatusjob.moc"
