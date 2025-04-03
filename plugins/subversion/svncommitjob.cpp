/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svncommitjob.h"
#include "svncommitjob_p.h"

#include <QMutexLocker>

#include <ThreadWeaver/ThreadWeaver>
#include <ThreadWeaver/Weaver>
#include <KLocalizedString>

#include <vector>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/targets.hpp"

#include <iostream>

#include "svninternaljobbase.h"
#include "kdevsvnplugin.h"

SvnInternalCommitJob::SvnInternalCommitJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalCommitJob::setRecursive( bool recursive )
{
    QMutexLocker l( &m_mutex );
    m_recursive = recursive;
}

void SvnInternalCommitJob::setCommitMessage( const QString& msg )
{
    QMutexLocker l( &m_mutex );
    m_commitMessage = msg;
}

void SvnInternalCommitJob::setUrls( const QList<QUrl>& urls )
{
    QMutexLocker l( &m_mutex );
    m_urls = urls;
}

void SvnInternalCommitJob::setKeepLock( bool lock )
{
    QMutexLocker l( &m_mutex );
    m_keepLock = lock;
}

QList<QUrl> SvnInternalCommitJob::urls() const
{
    QMutexLocker l( &m_mutex );
    return m_urls;
}

QString SvnInternalCommitJob::commitMessage() const
{
    QMutexLocker l( &m_mutex );
    return m_commitMessage;
}

bool SvnInternalCommitJob::recursive() const
{
    QMutexLocker l( &m_mutex );
    return m_recursive;
}

bool SvnInternalCommitJob::keepLock() const
{
    QMutexLocker l( &m_mutex );
    return m_keepLock;
}


void SvnInternalCommitJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();
    svn::Client cli(m_ctxt);
    std::vector<svn::Path> targets;
    const QList<QUrl> l = urls();
    for (const QUrl& u : l) {
        QByteArray path = u.toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        targets.push_back( svn::Path( path.data() ) );
    }
    QByteArray ba = commitMessage().toUtf8();
    try
    {
        cli.commit( svn::Targets(targets), ba.data(), recursive(), keepLock() );
    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Couldn't commit:" << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

SvnCommitJob::SvnCommitJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl(parent, KDevelop::OutputJob::Verbose )
{
    setType( KDevelop::VcsJob::Commit );
    setObjectName(i18n("Subversion Commit"));
}

QVariant SvnCommitJob::fetchResults()
{
    return QVariant();
}

void SvnCommitJob::start()
{
    if( m_job->urls().isEmpty() ) {
        failToStart(i18n("Not enough information to execute commit"));
    } else {
        startInternalJob(i18n("Committing..."));
    }
}

void SvnCommitJob::setCommitMessage( const QString& msg )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setCommitMessage( msg );
}

void SvnCommitJob::setKeepLock( bool keepLock )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setKeepLock( keepLock );
}

void SvnCommitJob::setUrls( const QList<QUrl>& urls )
{
    qCDebug(PLUGIN_SVN) << "Setting urls?" <<  status() << urls;
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setUrls( urls );
}

void SvnCommitJob::setRecursive( bool recursive )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setRecursive( recursive );
}

#include "moc_svncommitjob_p.cpp"
#include "moc_svncommitjob.cpp"
