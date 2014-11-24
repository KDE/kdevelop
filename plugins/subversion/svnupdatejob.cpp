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

#include "svnupdatejob.h"
#include "svnupdatejob_p.h"

#include <QMutexLocker>
#include <QDateTime>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/targets.hpp"


SvnInternalUpdateJob::SvnInternalUpdateJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
    , m_recursive( false )
    , m_ignoreExternals( false )
{
}

void SvnInternalUpdateJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
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
        svn::Revision rev = createSvnCppRevisionFromVcsRevision( m_revision );
        if( rev.kind() == svn_opt_revision_unspecified )
        {
            m_success = false;
            return;
        }
        cli.update( svn::Targets( targets ), rev, recursive(), ignoreExternals() );
    }catch( svn::ClientException ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while updating files: "
                << m_locations
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalUpdateJob::setRecursive( bool recursive )
{
    QMutexLocker l( m_mutex );
    m_recursive = recursive;
}

void SvnInternalUpdateJob::setLocations( const QList<QUrl>& urls )
{
    QMutexLocker l( m_mutex );
    m_locations = urls;
}


void SvnInternalUpdateJob::setIgnoreExternals( bool ignore )
{
    QMutexLocker l( m_mutex );
    m_ignoreExternals = ignore;
}

bool SvnInternalUpdateJob::ignoreExternals() const
{
    QMutexLocker l( m_mutex );
    return m_ignoreExternals;
}

void SvnInternalUpdateJob::setRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( m_mutex );
    m_revision = rev;
}

QList<QUrl> SvnInternalUpdateJob::locations() const
{
    QMutexLocker l( m_mutex );
    return m_locations;
}

KDevelop::VcsRevision SvnInternalUpdateJob::revision() const
{
    QMutexLocker l( m_mutex );
    return m_revision;
}

bool SvnInternalUpdateJob::recursive() const
{
    QMutexLocker l( m_mutex );
    return m_recursive;
}

SvnUpdateJob::SvnUpdateJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( new SvnInternalUpdateJob(this), parent, KDevelop::OutputJob::Verbose )
{
    setType( KDevelop::VcsJob::Add );
    setObjectName(i18n("Subversion Update"));
}

QVariant SvnUpdateJob::fetchResults()
{
    return QVariant();
}

void SvnUpdateJob::start()
{
    if( m_job->locations().isEmpty() )
    {
        internalJobFailed();
        setErrorText( i18n( "Not enough Information to execute update" ) );
    }else
    {
        qCDebug(PLUGIN_SVN) << "updating urls:" << m_job->locations();
        startInternalJob();
    }
}

void SvnUpdateJob::setLocations( const QList<QUrl>& urls )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocations( urls );
}

void SvnUpdateJob::setRecursive( bool recursive )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setRecursive( recursive );
}

void SvnUpdateJob::setRevision( const KDevelop::VcsRevision& rev )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setRevision( rev );
}

void SvnUpdateJob::setIgnoreExternals( bool ignore )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setIgnoreExternals( ignore );
}
