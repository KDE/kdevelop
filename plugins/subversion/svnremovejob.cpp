/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnremovejob.h"
#include "svnremovejob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/targets.hpp"

SvnInternalRemoveJob::SvnInternalRemoveJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalRemoveJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
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
        cli.remove( svn::Targets( targets ), force() );

    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while removing files: "
                << m_locations
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalRemoveJob::setLocations( const QList<QUrl>& urls )
{
    QMutexLocker l( &m_mutex );
    m_locations = urls;
}

QList<QUrl> SvnInternalRemoveJob::locations() const
{
    QMutexLocker l( &m_mutex );
    return m_locations;
}

void SvnInternalRemoveJob::setForce( bool force )
{
    QMutexLocker l( &m_mutex );
    m_force = force;
}

bool SvnInternalRemoveJob::force() const
{
    QMutexLocker l( &m_mutex );
    return m_force;
}

SvnRemoveJob::SvnRemoveJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Add );
    setObjectName(i18n("Subversion Remove"));
}

QVariant SvnRemoveJob::fetchResults()
{
    return QVariant();
}

void SvnRemoveJob::start()
{
    if( m_job->locations().isEmpty() ) {
        failToStart(i18n("Not enough information to execute remove job"));
    } else {
        qCDebug(PLUGIN_SVN) << "removing urls:" << m_job->locations();
        startInternalJob();
    }
}

void SvnRemoveJob::setLocations( const QList<QUrl>& urls )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocations( urls );
}

void SvnRemoveJob::setForce( bool force )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setForce( force );
}

#include "moc_svnremovejob.cpp"
#include "moc_svnremovejob_p.cpp"
