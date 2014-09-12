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
#include <QFileInfo>

#include <KLocalizedString>
#include <kdebug.h>
#include <ThreadWeaver.h>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"

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
        QMutexLocker l( m_mutex );
        QString srcdir = QFileInfo( m_sourceDirectory.toLocalFile() ).canonicalFilePath();
        QByteArray srcba = srcdir.toUtf8();
        KUrl desturl = KUrl( m_destinationRepository.repositoryServer() );
        desturl.cleanPath(KUrl::SimplifyDirSeparators);
        QByteArray destba = desturl.url( KUrl::RemoveTrailingSlash ).toUtf8();
        QByteArray msg = m_message.toUtf8();
        qDebug() << "Importing" << srcba << "into" << destba;
        cli.import( svn::Path( srcba.data() ), destba.data(), msg.data(), true );
    }catch( svn::ClientException ce )
    {
        kDebug() << "Exception while importing: "
                << m_sourceDirectory
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

bool SvnImportInternalJob::isValid() const
{
    return !m_message.isEmpty() && m_sourceDirectory.isLocalFile() && QFileInfo( m_sourceDirectory.toLocalFile() ).exists() && !m_destinationRepository.repositoryServer().isEmpty();
}

KUrl SvnImportInternalJob::source() const
{
    QMutexLocker l( m_mutex );
    return m_sourceDirectory;
}

void SvnImportInternalJob::setMessage( const QString& message )
{
    QMutexLocker l( m_mutex );
    m_message = message;
}

void SvnImportInternalJob::setMapping( const KUrl & sourceDirectory, const KDevelop::VcsLocation & destinationRepository)
{
    QMutexLocker l( m_mutex );
    m_sourceDirectory = sourceDirectory;
    m_destinationRepository = destinationRepository;
}

QString SvnImportInternalJob::message() const
{
    QMutexLocker l( m_mutex );
    return m_message;
}

SvnImportJob::SvnImportJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Import );
    m_job = new SvnImportInternalJob( this );
    setObjectName(i18n("Subversion Import"));
}

QVariant SvnImportJob::fetchResults()
{
    return QVariant();
}

void SvnImportJob::start()
{
    if( !m_job->isValid() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to import" ) );
    }else
    {
        kDebug() << "importing:" << m_job->source();
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnImportJob::internalJob() const
{
    return m_job;
}

void SvnImportJob::setMapping( const KUrl & sourceDirectory, const KDevelop::VcsLocation & destinationRepository)
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setMapping( sourceDirectory, destinationRepository);
}

void SvnImportJob::setMessage( const QString& msg )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setMessage( msg );
}
