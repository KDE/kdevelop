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

#include "svncopyjob.h"
#include "svncopyjob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"

SvnInternalCopyJob::SvnInternalCopyJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalCopyJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    try
    {
        QByteArray srcba = sourceLocation().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        QByteArray dstba = destinationLocation().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        cli.copy( svn::Path( srcba.data() ), svn::Revision(), svn::Path( dstba.data() ) );
    }catch( svn::ClientException ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while copying file: "
                << sourceLocation() << "to" << destinationLocation()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalCopyJob::setDestinationLocation( const QUrl &url )
{
    QMutexLocker l( m_mutex );
    m_destinationLocation = url;
}

QUrl SvnInternalCopyJob::destinationLocation() const
{
    QMutexLocker l( m_mutex );
    return m_destinationLocation;
}

void SvnInternalCopyJob::setSourceLocation( const QUrl &url )
{
    QMutexLocker l( m_mutex );
    m_sourceLocation = url;
}

QUrl SvnInternalCopyJob::sourceLocation() const
{
    QMutexLocker l( m_mutex );
    return m_sourceLocation;
}

SvnCopyJob::SvnCopyJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( new SvnInternalCopyJob(this), parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Copy );
    setObjectName(i18n("Subversion Copy"));
}

QVariant SvnCopyJob::fetchResults()
{
    return QVariant();
}

void SvnCopyJob::start()
{
    if ( m_job->sourceLocation().isEmpty() || m_job->destinationLocation().isEmpty() ) {
        internalJobFailed();
        setErrorText( i18n( "Not enough information to copy file" ) );
    } else {
        qCDebug(PLUGIN_SVN) << "copying url:" << m_job->sourceLocation() << "to url" << m_job->destinationLocation();
        startInternalJob();
    }
}

void SvnCopyJob::setDestinationLocation( const QUrl &url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setDestinationLocation( url );
}

void SvnCopyJob::setSourceLocation( const QUrl &url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setSourceLocation( url );
}
