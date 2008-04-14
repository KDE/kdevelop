/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                        *
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

#include "svnimportjob.h"
#include "svnimportjob_p.h"

#include <QMutexLocker>

#include <klocale.h>
#include <kdebug.h>
#include <ThreadWeaver.h>

#include <kdevsvncpp/client.hpp>
#include <kdevsvncpp/path.hpp>

#include <vcs/vcslocation.h>

SvnImportInternalJob::SvnImportInternalJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnImportInternalJob::run()
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    try
    {
        KDevelop::VcsMapping map = mapping();
        KDevelop::VcsLocation src = map.sourceLocations().first();
        KDevelop::VcsLocation dest = map.destinationLocation( src );
        bool recurse = ( map.mappingFlag( src ) == KDevelop::VcsMapping::Recursive );
        QByteArray srcba = src.localUrl().toLocalFile().toUtf8();
        QByteArray destba = dest.repositoryServer().toUtf8();
        QByteArray msg = message().toUtf8();
        cli.import( svn::Path( srcba.data() ), destba.data(), msg.data(), recurse );
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while importing: "
                << mapping().sourceLocations().first().localUrl()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnImportInternalJob::setMessage( const QString& message )
{
    QMutexLocker l( m_mutex );
    m_message = message;
}

void SvnImportInternalJob::setMapping( const KDevelop::VcsMapping& mapping )
{
    QMutexLocker l( m_mutex );
    m_mapping = mapping;
}

KDevelop::VcsMapping SvnImportInternalJob::mapping() const
{
    QMutexLocker l( m_mutex );
    return m_mapping;
}
QString SvnImportInternalJob::message() const
{
    QMutexLocker l( m_mutex );
    return m_message;
}

SvnImportJob::SvnImportJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent )
{
    setType( KDevelop::VcsJob::Import );
    m_job = new SvnImportInternalJob( this );
}

QVariant SvnImportJob::fetchResults()
{
    return QVariant();
}

void SvnImportJob::start()
{
    if( m_job->mapping().sourceLocations().isEmpty() || !m_job->mapping().sourceLocations().first().localUrl().isValid() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to import" ) );
    }else
    {
        kDebug(9510) << "importing:" << m_job->mapping().sourceLocations().first().localUrl();
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnImportJob::internalJob() const
{
    return m_job;
}

void SvnImportJob::setMapping( const KDevelop::VcsMapping& mapping )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setMapping( mapping );
}

void SvnImportJob::setMessage( const QString& msg )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setMessage( msg );
}


#include "svnimportjob.moc"
#include "svnimportjob_p.moc"
