/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnupdatejob.h"
#include "svnupdatejob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/targets.hpp"


SvnInternalUpdateJob::SvnInternalUpdateJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalUpdateJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    std::vector<svn::Path> targets;
    const QList<QUrl> l = locations();
    for (const QUrl& url : l) {
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
    }catch( const svn::ClientException& ce )
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
    QMutexLocker l( &m_mutex );
    m_recursive = recursive;
}

void SvnInternalUpdateJob::setLocations( const QList<QUrl>& urls )
{
    QMutexLocker l( &m_mutex );
    m_locations = urls;
}


void SvnInternalUpdateJob::setIgnoreExternals( bool ignore )
{
    QMutexLocker l( &m_mutex );
    m_ignoreExternals = ignore;
}

bool SvnInternalUpdateJob::ignoreExternals() const
{
    QMutexLocker l( &m_mutex );
    return m_ignoreExternals;
}

void SvnInternalUpdateJob::setRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( &m_mutex );
    m_revision = rev;
}

QList<QUrl> SvnInternalUpdateJob::locations() const
{
    QMutexLocker l( &m_mutex );
    return m_locations;
}

KDevelop::VcsRevision SvnInternalUpdateJob::revision() const
{
    QMutexLocker l( &m_mutex );
    return m_revision;
}

bool SvnInternalUpdateJob::recursive() const
{
    QMutexLocker l( &m_mutex );
    return m_recursive;
}

SvnUpdateJob::SvnUpdateJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl(parent, KDevelop::OutputJob::Verbose )
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
        failToStart(i18n("Not enough Information to execute update"));
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

#include "moc_svnupdatejob_p.cpp"
#include "moc_svnupdatejob.cpp"
