/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnimportjob.h"
#include "svnimportjob_p.h"

#include <QMutexLocker>
#include <QFileInfo>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"

#include <vcs/vcslocation.h>

SvnImportInternalJob::SvnImportInternalJob(SvnJobBase* parent)
    : SvnInternalJobBase(parent)
{
}

void SvnImportInternalJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    try
    {
        QMutexLocker l( &m_mutex );
        QString srcdir = QFileInfo( m_sourceDirectory.toLocalFile() ).canonicalFilePath();
        QByteArray srcba = srcdir.toUtf8();
        QUrl dest = QUrl::fromUserInput( m_destinationRepository.repositoryServer() );
        QByteArray destba = dest.url(QUrl::NormalizePathSegments).toUtf8();
        QByteArray msg = m_message.toUtf8();
        qCDebug(PLUGIN_SVN) << "Importing" << srcba << "into" << destba;
        cli.import( svn::Path( srcba.data() ), destba.data(), msg.data(), true );
    }catch( const svn::ClientException& ce )
    {
        qCWarning(PLUGIN_SVN) << "Exception while importing: "
                << m_sourceDirectory
                << ce.message();
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }

    qDebug() << "finished";
}

bool SvnImportInternalJob::isValid() const
{
    return !m_message.isEmpty() && m_sourceDirectory.isLocalFile() && QFileInfo::exists( m_sourceDirectory.toLocalFile() ) && !m_destinationRepository.repositoryServer().isEmpty();
}

QUrl SvnImportInternalJob::source() const
{
    QMutexLocker l( &m_mutex );
    return m_sourceDirectory;
}

void SvnImportInternalJob::setMessage( const QString& message )
{
    QMutexLocker l( &m_mutex );
    m_message = message;
}

void SvnImportInternalJob::setMapping( const QUrl &sourceDirectory, const KDevelop::VcsLocation & destinationRepository)
{
    QMutexLocker l( &m_mutex );
    m_sourceDirectory = sourceDirectory;
    m_destinationRepository = destinationRepository;
}

QString SvnImportInternalJob::message() const
{
    QMutexLocker l( &m_mutex );
    return m_message;
}

SvnImportJob::SvnImportJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Import );
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
        failToStart(i18n("Not enough information to import"));
    }else
    {
        qCDebug(PLUGIN_SVN) << "importing:" << m_job->source();
        startInternalJob();
    }
}

void SvnImportJob::setMapping( const QUrl &sourceDirectory, const KDevelop::VcsLocation & destinationRepository)
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setMapping( sourceDirectory, destinationRepository);
}

void SvnImportJob::setMessage( const QString& msg )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setMessage( msg );
}

#include "moc_svnimportjob.cpp"
#include "moc_svnimportjob_p.cpp"
