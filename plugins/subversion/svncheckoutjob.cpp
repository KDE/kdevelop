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

#include "svncheckoutjob.h"
#include "svncheckoutjob_p.h"

#include <QMutexLocker>

#include <klocale.h>
#include <kdebug.h>
#include <ThreadWeaver.h>

#include <svncpp/client.hpp>
#include <svncpp/path.hpp>

#include <vcs/vcslocation.h>

SvnInternalCheckoutJob::SvnInternalCheckoutJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalCheckoutJob::run()
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    try
    {
        KDevelop::VcsMapping map = mapping();
        KDevelop::VcsLocation src = map.sourceLocations().first();
        KDevelop::VcsLocation dest = map.destinationLocation( src );
        bool recurse = ( map.mappingFlag( src ) == KDevelop::VcsMapping::Recursive );
        QByteArray srcba = src.repositoryServer().toUtf8();
        QByteArray destba = dest.localUrl().toLocalFile().toUtf8();
        kDebug(9510) << srcba << destba;
        cli.checkout( srcba.data(), svn::Path( destba.data() ), svn::Revision(svn::Revision::HEAD), recurse );
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while checking out: "
                << mapping().sourceLocations().first().repositoryServer()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalCheckoutJob::setMapping( const KDevelop::VcsMapping& mapping )
{
    QMutexLocker l( m_mutex );
    m_mapping = mapping;
}

KDevelop::VcsMapping SvnInternalCheckoutJob::mapping() const
{
    QMutexLocker l( m_mutex );
    return m_mapping;
}

SvnCheckoutJob::SvnCheckoutJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent )
{
    setType( KDevelop::VcsJob::Import );
    m_job = new SvnInternalCheckoutJob( this );
}

QVariant SvnCheckoutJob::fetchResults()
{
    return QVariant();
}

void SvnCheckoutJob::start()
{
    if( m_job->mapping().sourceLocations().isEmpty() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to checkout" ) );
    }else
    {
        kDebug(9510) << "checking out:" << m_job->mapping().sourceLocations().first().repositoryServer();
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnCheckoutJob::internalJob() const
{
    return m_job;
}

void SvnCheckoutJob::setMapping( const KDevelop::VcsMapping& mapping )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setMapping( mapping );
}


#include "svncheckoutjob.moc"
#include "svncheckoutjob_p.moc"

