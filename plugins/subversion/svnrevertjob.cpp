/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnrevertjob.h"
#include "svnrevertjob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/targets.hpp"

SvnInternalRevertJob::SvnInternalRevertJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalRevertJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
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
        cli.revert( svn::Targets( targets ), recursive() );
    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while reverting files: "
                << m_locations
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalRevertJob::setRecursive( bool recursive )
{
    QMutexLocker l( &m_mutex );
    m_recursive = recursive;
}

void SvnInternalRevertJob::setLocations( const QList<QUrl>& urls )
{
    QMutexLocker l( &m_mutex );
    m_locations = urls;
}

QList<QUrl> SvnInternalRevertJob::locations() const
{
    QMutexLocker l( &m_mutex );
    return m_locations;
}
bool SvnInternalRevertJob::recursive() const
{
    QMutexLocker l( &m_mutex );
    return m_recursive;
}

SvnRevertJob::SvnRevertJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Add );
    setObjectName(i18n("Subversion Revert"));
}

QVariant SvnRevertJob::fetchResults()
{
    return QVariant();
}

void SvnRevertJob::start()
{
    if (m_job->locations().isEmpty()) {
        failToStart(i18n("Not enough information to execute revert"));
    } else {
        startInternalJob();
    }
}

void SvnRevertJob::setLocations( const QList<QUrl>& urls )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocations( urls );
}

void SvnRevertJob::setRecursive( bool recursive )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setRecursive( recursive );
}

#include "moc_svnrevertjob.cpp"
#include "moc_svnrevertjob_p.cpp"
