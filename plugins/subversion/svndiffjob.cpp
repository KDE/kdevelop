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

#include "svndiffjob.h"
#include "svndiffjob_p.h"

#include <QMutexLocker>
#include <QWaitCondition>
#include <QRegExp>
#include <QStringList>
#include <QFileInfo>

#include <kdebug.h>
#include <KLocalizedString>
#include <ThreadWeaver.h>

#include <vcs/vcsrevision.h>

#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/revision.hpp"

#include "icore.h"
#include "iruncontroller.h"

#include "svnclient.h"
#include "svncatjob.h"

///@todo The subversion library returns borked diffs, where the headers are at the end. This function
///           takes those headers, and moves them into the correct place to create a valid working diff.
///           Find the source of this problem.
QString repairDiff(QString diff) {
    kDebug() << "diff before repair:" << diff;
    QStringList lines = diff.split('\n');
    QMap<QString, QString> headers;
    for(int a = 0; a < lines.size()-1; ++a) {
        if(lines[a].startsWith("Index: ") && lines[a+1].startsWith("=====")) {
            QString fileName = lines[a].mid(strlen("Index: ")).trimmed();
            headers[fileName] = lines[a];
            kDebug() << "found header for" << fileName;
            lines[a] = QString();
            if(lines[a+1].startsWith("======")) {
                headers[fileName] += '\n' + lines[a+1];
            lines[a+1] = QString();
            }
        }
    }
    
    QRegExp spaceRegExp("\\s");
    
    for(int a = 0; a < lines.size()-1; ++a) {
        if(lines[a].startsWith("--- ")) {
            QString tail = lines[a].mid(strlen("--- "));
            if(tail.indexOf(spaceRegExp) != -1) {
                QString file = tail.left(tail.indexOf(spaceRegExp));
                kDebug() << "checking for" << file;
                if(headers.contains(file)) {
                    kDebug() << "adding header for" << file << ":" << headers[file];
                    lines[a] = headers[file] + '\n' + lines[a];
                }
            }
        }
    }
    QString ret = lines.join("\n");
    kDebug() << "repaired diff:" << ret;
    return ret;
}

//@TODO: Handle raw diffs by using SvnCatJob to fetch both files/revisions

SvnInternalDiffJob::SvnInternalDiffJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent ), m_recursive( true ),
      m_ignoreAncestry( false ), m_ignoreContentType( false ),
      m_noDiffOnDelete( false )
{
    m_pegRevision.setRevisionValue( KDevelop::VcsRevision::Head,
                                    KDevelop::VcsRevision::Special );
}

void SvnInternalDiffJob::run()
{
    initBeforeRun();

    SvnClient cli(m_ctxt);
    try
    {

        QString diff;
        if( destination().isValid() )
        {
            QByteArray srcba;
            if( source().type() == KDevelop::VcsLocation::LocalLocation )
            {
                KUrl url = source().localUrl();
                if( url.isLocalFile() )
                {
                    srcba = url.toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
                }else
                {
                    srcba = url.url( KUrl::RemoveTrailingSlash ).toUtf8();
                }
            }else
            {
                srcba = source().repositoryServer().toUtf8();
            }
            QByteArray dstba;
            if( destination().type() == KDevelop::VcsLocation::LocalLocation )
            {
                KUrl url = destination().localUrl();
                if( url.isLocalFile() )
                {
                    dstba = url.toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
                }else
                {
                    dstba = url.url().toUtf8();
                }
            }else
            {
                dstba = destination().repositoryServer().toUtf8();
            }
            svn::Revision srcRev = createSvnCppRevisionFromVcsRevision( srcRevision() );
            svn::Revision dstRev = createSvnCppRevisionFromVcsRevision( dstRevision() );
            if( srcba.isEmpty() || ( dstba.isEmpty() && srcRev.kind() == svn_opt_revision_unspecified
                && dstRev.kind() == svn_opt_revision_unspecified ) )
            {
                throw svn::ClientException( "Not enough information for a diff");
            }
            diff = cli.diff( svn::Path( srcba.data() ), srcRev, svn::Path( dstba.data() ),
                             dstRev, recursive(), ignoreAncestry(),
                             noDiffOnDelete(), ignoreContentType() );
        }else
        {
            QByteArray srcba;
            if( source().type() == KDevelop::VcsLocation::LocalLocation )
            {
                KUrl url = source().localUrl();
                if( url.isLocalFile() )
                {
                    srcba = url.toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
                }else
                {
                    srcba = url.url().toUtf8();
                }
            }else
            {
                srcba = source().repositoryServer().toUtf8();
            }
            svn::Revision pegRev = createSvnCppRevisionFromVcsRevision( pegRevision() );
            svn::Revision srcRev = createSvnCppRevisionFromVcsRevision( srcRevision() );
            svn::Revision dstRev = createSvnCppRevisionFromVcsRevision( dstRevision() );
            if( srcba.isEmpty() || pegRev.kind() == svn_opt_revision_unspecified
                || dstRev.kind() == svn_opt_revision_unspecified
                || srcRev.kind() == svn_opt_revision_unspecified)
            {
                throw svn::ClientException( "Not enough information for a diff");
            }
            diff = cli.diff( svn::Path( srcba.data() ), pegRev, srcRev,
                             dstRev, recursive(), ignoreAncestry(),
                             noDiffOnDelete(), ignoreContentType() );
        }
        diff = repairDiff(diff);
        emit gotDiff( diff );

    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while doing a diff: "
                << m_source.localUrl() << m_source.repositoryServer() << m_srcRevision.prettyValue()
                << m_destination.localUrl() << m_destination.repositoryServer() << m_dstRevision.prettyValue()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalDiffJob::setSource( const KDevelop::VcsLocation& src )
{
    QMutexLocker l( m_mutex );
    m_source = src;
}
void SvnInternalDiffJob::setDestination( const KDevelop::VcsLocation& dst )
{
    QMutexLocker l( m_mutex );
    m_destination = dst;
}
void SvnInternalDiffJob::setSrcRevision( const KDevelop::VcsRevision& srcRev )
{
    QMutexLocker l( m_mutex );
    m_srcRevision = srcRev;
}
void SvnInternalDiffJob::setDstRevision( const KDevelop::VcsRevision& dstRev )
{
    QMutexLocker l( m_mutex );
    m_dstRevision = dstRev;
}
void SvnInternalDiffJob::setPegRevision( const KDevelop::VcsRevision& pegRev )
{
    QMutexLocker l( m_mutex );
    m_pegRevision = pegRev;
}
void SvnInternalDiffJob::setRecursive( bool recursive )
{
    QMutexLocker l( m_mutex );
    m_recursive = recursive;
}
void SvnInternalDiffJob::setIgnoreAncestry( bool ignoreAncestry )
{
    QMutexLocker l( m_mutex );
    m_ignoreAncestry = ignoreAncestry;
}
void SvnInternalDiffJob::setIgnoreContentType( bool ignoreContentType )
{
    QMutexLocker l( m_mutex );
    m_ignoreContentType = ignoreContentType;
}
void SvnInternalDiffJob::setNoDiffOnDelete( bool noDiffOnDelete )
{
    QMutexLocker l( m_mutex );
    m_noDiffOnDelete = noDiffOnDelete;
}

bool SvnInternalDiffJob::recursive() const
{
    QMutexLocker l( m_mutex );
    return m_recursive;
}
bool SvnInternalDiffJob::ignoreAncestry() const
{
    QMutexLocker l( m_mutex );
    return m_ignoreAncestry;
}
bool SvnInternalDiffJob::ignoreContentType() const
{
    QMutexLocker l( m_mutex );
    return m_ignoreContentType;
}
bool SvnInternalDiffJob::noDiffOnDelete() const
{
    QMutexLocker l( m_mutex );
    return m_noDiffOnDelete;
}
KDevelop::VcsLocation SvnInternalDiffJob::source() const
{
    QMutexLocker l( m_mutex );
    return m_source;
}
KDevelop::VcsLocation SvnInternalDiffJob::destination() const
{
    QMutexLocker l( m_mutex );
    return m_destination;
}
KDevelop::VcsRevision SvnInternalDiffJob::srcRevision() const
{
    QMutexLocker l( m_mutex );
    return m_srcRevision;
}
KDevelop::VcsRevision SvnInternalDiffJob::dstRevision() const
{
    QMutexLocker l( m_mutex );
    return m_dstRevision;
}
KDevelop::VcsRevision SvnInternalDiffJob::pegRevision() const
{
    QMutexLocker l( m_mutex );
    return m_pegRevision;
}

SvnDiffJob::SvnDiffJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Add );
    m_job = new SvnInternalDiffJob( this );

    setObjectName(i18n("Subversion Diff"));
}

QVariant SvnDiffJob::fetchResults()
{
    return qVariantFromValue( m_diff );
}

void SvnDiffJob::start()
{

    disconnect( m_job, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(internalJobDone(ThreadWeaver::Job*)) );
    if( !m_job->source().isValid()
         || ( !m_job->destination().isValid() &&
                ( m_job->srcRevision().revisionType() == KDevelop::VcsRevision::Invalid
                 || m_job->dstRevision().revisionType() == KDevelop::VcsRevision::Invalid ) )
      )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information given to execute diff" ) );
    }else
    {
        connect( m_job, SIGNAL(gotDiff(QString)),
                 this, SLOT(setDiff(QString)),
                 Qt::QueuedConnection );
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnDiffJob::internalJob() const
{
    return m_job;
}

void SvnDiffJob::setSource( const KDevelop::VcsLocation& source )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setSource( source );
}
void SvnDiffJob::setDestination( const KDevelop::VcsLocation& destination )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setDestination( destination );
}
void SvnDiffJob::setPegRevision( const KDevelop::VcsRevision& pegRevision )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setPegRevision( pegRevision );
}

void SvnDiffJob::setSrcRevision( const KDevelop::VcsRevision& srcRevision )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setSrcRevision( srcRevision );
}
void SvnDiffJob::setDstRevision( const KDevelop::VcsRevision& dstRevision )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setDstRevision( dstRevision );
}
void SvnDiffJob::setRecursive( bool recursive )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setRecursive( recursive );
}
void SvnDiffJob::setIgnoreAncestry( bool ignoreAncestry )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setIgnoreAncestry( ignoreAncestry );
}
void SvnDiffJob::setIgnoreContentType( bool ignoreContentType )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setIgnoreContentType( ignoreContentType );
}
void SvnDiffJob::setNoDiffOnDelete( bool noDiffOnDelete )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setNoDiffOnDelete( noDiffOnDelete );
}

void SvnDiffJob::setDiff( const QString& diff )
{
    m_diff = KDevelop::VcsDiff();
    m_diff.setBaseDiff(KUrl("/"));
    m_diff.setType( KDevelop::VcsDiff::DiffUnified );

    m_diff.setContentType( KDevelop::VcsDiff::Text );
    m_diff.setDiff( diff );

    QRegExp fileRe("(?:^|\n)Index: ([^\n]+)\n");

    QStringList paths;
    int pos = 0;

    while( ( pos = fileRe.indexIn( diff, pos ) ) != -1 )
    {
        paths << fileRe.cap(1);
        pos += fileRe.matchedLength();
    }

    if (paths.isEmpty()) {
        internalJobDone( m_job );
        emit resultsReady( this );
        return;
    }

    foreach( const QString &s, paths )
    {
        if( !s.isEmpty() )
        {
            SvnCatJob* job = new SvnCatJob( m_part );
            KDevelop::VcsLocation l = m_job->source();
            if( l.type() == KDevelop::VcsLocation::LocalLocation )
            {
                l.setLocalUrl( KUrl( s ) );
            }else
            {
                QString repoLocation = KUrl( l.repositoryServer() ).toLocalFile( KUrl::RemoveTrailingSlash );
                QFileInfo fi( repoLocation );
                if( s == fi.fileName() )
                {
                    l.setRepositoryServer( l.repositoryServer() );
                }else
                {
                    l.setRepositoryServer( l.repositoryServer() + '/' + s );
                }
            }

            job->setSource( l );
            job->setPegRevision( m_job->pegRevision() );
            job->setSrcRevision( m_job->srcRevision() );

            m_catJobMap[job] = l;

            connect( job, SIGNAL(resultsReady(KDevelop::VcsJob*)), this, SLOT(addLeftText(KDevelop::VcsJob*)) );
            connect( job, SIGNAL(result(KJob*)), this, SLOT(removeJob(KJob*)) );

            KDevelop::ICore::self()->runController()->registerJob(job);
        }
    }
}

void SvnDiffJob::addLeftText( KDevelop::VcsJob* job )
{
    if( m_catJobMap.contains( job ) )
    {
        QVariant v = job->fetchResults();
        m_diff.addLeftText( m_catJobMap[job], v.toString() );
        m_catJobMap.remove(job);
        // KJobs delete themselves when finished
    }
    if( m_catJobMap.isEmpty() )
    {
        internalJobDone( m_job );
        emit resultsReady( this );
    }
}

void SvnDiffJob::removeJob( KJob* job )
{
    if( job->error() != 0 )
    {
        KDevelop::VcsJob* j = dynamic_cast<KDevelop::VcsJob*>( job );
        if( j )
        {
            if( m_catJobMap.contains( j ) )
            {
                m_catJobMap.remove(j);
                // KJobs delete themselves when finished
            }
        }
    }

    if( m_catJobMap.isEmpty() )
    {
        internalJobDone( m_job );
        emit resultsReady( this );
    }
}

void SvnDiffJob::setDiffType( KDevelop::VcsDiff::Type type )
{
    m_diffType = type;
}
