/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svncheckoutjob.h"
#include "svncheckoutjob_p.h"

#include <QFileInfo>
#include <QMutexLocker>

#include <KIO/Global>
#include <KLocalizedString>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/path.hpp"

#include <vcs/vcslocation.h>
#include <util/path.h>

SvnInternalCheckoutJob::SvnInternalCheckoutJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

bool SvnInternalCheckoutJob::isValid() const
{
    QMutexLocker l( &m_mutex );
    return m_sourceRepository.isValid() && m_destinationDirectory.isLocalFile() && QFileInfo::exists(KIO::upUrl(m_destinationDirectory).toLocalFile());
}

void SvnInternalCheckoutJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    svn::Client cli(m_ctxt);
    try {
        bool recurse = ( recursion() == KDevelop::IBasicVersionControl::Recursive );
        QUrl desturl = QUrl( source().repositoryServer() ).adjusted(QUrl::StripTrailingSlash | QUrl::NormalizePathSegments  );
        const QByteArray srcba = desturl.url().toUtf8();
        KDevelop::Path destdir(KDevelop::Path(destination()).parent(), destination().fileName());
        QByteArray destba = destdir.toLocalFile().toUtf8();
        qCDebug(PLUGIN_SVN) << srcba << destba << recurse;
        cli.checkout( srcba.data(), svn::Path( destba.data() ), svn::Revision::HEAD, recurse );
    } catch( const svn::ClientException& ce ) {
        qCDebug(PLUGIN_SVN) << "Exception while checking out: " << source().repositoryServer() << ce.message();
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalCheckoutJob::setMapping( const KDevelop::VcsLocation & sourceRepository, const QUrl & destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    QMutexLocker l( &m_mutex );
    m_sourceRepository = sourceRepository;
    m_destinationDirectory = destinationDirectory;
    m_recursion = recursion;
}

KDevelop::VcsLocation SvnInternalCheckoutJob::source() const
{
    QMutexLocker l( &m_mutex );
    return m_sourceRepository;
}

KDevelop::IBasicVersionControl::RecursionMode SvnInternalCheckoutJob::recursion() const
{
    QMutexLocker l( &m_mutex );
    return m_recursion;
}

QUrl SvnInternalCheckoutJob::destination() const
{
    QMutexLocker l( &m_mutex );
    return m_destinationDirectory;
}

SvnCheckoutJob::SvnCheckoutJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Import );
    setObjectName(i18n("Subversion Checkout"));
}

QVariant SvnCheckoutJob::fetchResults()
{
    return QVariant();
}

void SvnCheckoutJob::start()
{
    if (!m_job->isValid() ) {
        failToStart(i18n("Not enough information to checkout"));
    } else {
        qCDebug(PLUGIN_SVN) << "checking out: " << m_job->source().repositoryServer();
        startInternalJob();
    }
}

void SvnCheckoutJob::setMapping( const KDevelop::VcsLocation & sourceRepository, const QUrl & destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    if( status() == KDevelop::VcsJob::JobNotStarted ) {
        m_job->setMapping(sourceRepository, destinationDirectory, recursion);
    }
}

#include "moc_svncheckoutjob_p.cpp"
#include "moc_svncheckoutjob.cpp"
