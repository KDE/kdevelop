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

#include "svnremovejob.h"
#include "svnremovejob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>


#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/targets.hpp"

SvnInternalRemoveJob::SvnInternalRemoveJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
    , m_force( false )
{
}

void SvnInternalRemoveJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    std::vector<svn::Path> targets;
    QList<QUrl> l = locations();
    foreach( const QUrl &url, l )
    {
        QByteArray ba = url.toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        targets.push_back( svn::Path( ba.data() ) );
    }
    try
    {
        cli.remove( svn::Targets( targets ), force() );

    }catch( svn::ClientException ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while removing files: "
                << m_locations
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalRemoveJob::setLocations( const QList<QUrl>& urls )
{
    QMutexLocker l( m_mutex );
    m_locations = urls;
}

QList<QUrl> SvnInternalRemoveJob::locations() const
{
    QMutexLocker l( m_mutex );
    return m_locations;
}

void SvnInternalRemoveJob::setForce( bool force )
{
    QMutexLocker l( m_mutex );
    m_force = force;
}

bool SvnInternalRemoveJob::force() const
{
    QMutexLocker l( m_mutex );
    return m_force;
}

SvnRemoveJob::SvnRemoveJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Add );
    m_job = new SvnInternalRemoveJob( this );
    setObjectName(i18n("Subversion Remove"));
}

QVariant SvnRemoveJob::fetchResults()
{
    return QVariant();
}

void SvnRemoveJob::start()
{
    if( m_job->locations().isEmpty() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to execute remove job" ) );
    }else
    {
        qCDebug(PLUGIN_SVN) << "removing urls:" << m_job->locations();
        m_part->jobQueue()->stream() << ThreadWeaver::make_job_raw( m_job );
    }
}

SvnInternalJobBase* SvnRemoveJob::internalJob() const
{
    return m_job;
}

void SvnRemoveJob::setLocations( const QList<QUrl>& urls )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocations( urls );
}


void SvnRemoveJob::setForce( bool force )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setForce( force );
}
