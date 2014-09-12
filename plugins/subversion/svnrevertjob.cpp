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

#include "svnrevertjob.h"
#include "svnrevertjob_p.h"

#include <QMutexLocker>

#include <kdebug.h>
#include <KLocalizedString>

#include <ThreadWeaver.h>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/targets.hpp"

SvnInternalRevertJob::SvnInternalRevertJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
    , m_recursive( false )
{
}

void SvnInternalRevertJob::run()
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    std::vector<svn::Path> targets;
    KUrl::List l = locations();
    foreach( const KUrl &url, l )
    {
        QByteArray ba = url.toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
        targets.push_back( svn::Path( ba.data() ) );
    }
    try
    {
        cli.revert( svn::Targets( targets ), recursive() );
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while reverting files: "
                << m_locations
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalRevertJob::setRecursive( bool recursive )
{
    QMutexLocker l( m_mutex );
    m_recursive = recursive;
}

void SvnInternalRevertJob::setLocations( const KUrl::List& urls )
{
    QMutexLocker l( m_mutex );
    m_locations = urls;
}

KUrl::List SvnInternalRevertJob::locations() const
{
    QMutexLocker l( m_mutex );
    return m_locations;
}
bool SvnInternalRevertJob::recursive() const
{
    QMutexLocker l( m_mutex );
    return m_recursive;
}

SvnRevertJob::SvnRevertJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Add );
    m_job = new SvnInternalRevertJob( this );
    setObjectName(i18n("Subversion Revert"));
}

QVariant SvnRevertJob::fetchResults()
{
    return QVariant();
}

void SvnRevertJob::start()
{
    if( m_job->locations().isEmpty() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to execute revert" ) );
    }else
    {
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnRevertJob::internalJob() const
{
    return m_job;
}

void SvnRevertJob::setLocations( const KUrl::List& urls )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocations( urls );
}

void SvnRevertJob::setRecursive( bool recursive )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setRecursive( recursive );
}
