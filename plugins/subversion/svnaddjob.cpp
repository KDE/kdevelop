/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnaddjob.h"
#include "svnaddjob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"


SvnInternalAddJob::SvnInternalAddJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )

{
}

void SvnInternalAddJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    const QList<QUrl> l = locations();
    for (const QUrl& url : l) {
        try
        {
            QByteArray ba = url.toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
            cli.add( svn::Path( ba.data() ), recursive() );
        }catch( const svn::ClientException& ce )
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
    QMutexLocker l( &m_mutex );
    m_recursive = recursive;
}

void SvnInternalAddJob::setLocations( const QList<QUrl>& urls )
{
    QMutexLocker l( &m_mutex );
    m_locations = urls;
}

QList<QUrl> SvnInternalAddJob::locations() const
{
    QMutexLocker l( &m_mutex );
    return m_locations;
}
bool SvnInternalAddJob::recursive() const
{
    QMutexLocker l( &m_mutex );
    return m_recursive;
}

SvnAddJob::SvnAddJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Add );
    setObjectName(i18n("Subversion Add"));
}

QVariant SvnAddJob::fetchResults()
{
    return QVariant();
}

void SvnAddJob::start()
{
    if ( m_job->locations().isEmpty() ) {
        failToStart(i18n("Not enough information to add file"));
    } else {
        qCDebug(PLUGIN_SVN) << "adding urls:" << m_job->locations();
        startInternalJob();
    }
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

#include "moc_svnaddjob.cpp"
#include "moc_svnaddjob_p.cpp"
