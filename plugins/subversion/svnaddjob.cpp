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

#include "svnaddjob.h"
#include "svnaddjob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"


SvnInternalAddJob::SvnInternalAddJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
    , m_recursive( false )
{
}

void SvnInternalAddJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    QList<QUrl> l = locations();
    foreach( const QUrl &url, l )
    {
        try
        {
            QByteArray ba = url.toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
            cli.add( svn::Path( ba.data() ), recursive() );
        }catch( svn::ClientException ce )
        {
            qCDebug(PLUGIN_SVN) << "Exception while adding file: "
                    << url
                    << QString::fromUtf8( ce.message() );
            setErrorMessage( QString::fromUtf8( ce.message() ) );
            m_success = false;
        }
    }
}

void SvnInternalAddJob::setRecursive( bool recursive )
{
    QMutexLocker l( m_mutex );
    m_recursive = recursive;
}

void SvnInternalAddJob::setLocations( const QList<QUrl>& urls )
{
    QMutexLocker l( m_mutex );
    m_locations = urls;
}

QList<QUrl> SvnInternalAddJob::locations() const
{
    QMutexLocker l( m_mutex );
    return m_locations;
}
bool SvnInternalAddJob::recursive() const
{
    QMutexLocker l( m_mutex );
    return m_recursive;
}

SvnAddJob::SvnAddJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Add );
    m_job = QSharedPointer<SvnInternalAddJob>::create( this );
    setObjectName(i18n("Subversion Add"));
}

QVariant SvnAddJob::fetchResults()
{
    return QVariant();
}

void SvnAddJob::start()
{
    if( m_job->locations().isEmpty() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to add file" ) );
    }else
    {
        qCDebug(PLUGIN_SVN) << "adding urls:" << m_job->locations();
        m_part->jobQueue()->stream() << m_job;
    }
}

QSharedPointer<SvnInternalJobBase> SvnAddJob::internalJob() const
{
    return m_job;
}

void SvnAddJob::setLocations( const QList<QUrl>& urls )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocations( urls );
}

void SvnAddJob::setRecursive( bool recursive )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setRecursive( recursive );
}
