/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svndiffjob.h"
#include "svndiffjob_p.h"

#include <QMutexLocker>
#include <QRegularExpression>
#include <QStringList>
#include <QFileInfo>

#include <KLocalizedString>

#include <vcs/vcsrevision.h>

#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/revision.hpp"

#include "icore.h"
#include "iruncontroller.h"

#include "svnclient.h"

namespace {
///@todo The subversion library returns borked diffs, where the headers are at the end. This function
///           takes those headers, and moves them into the correct place to create a valid working diff.
///           Find the source of this problem.
QString repairDiff(const QString& diff) {
    qCDebug(PLUGIN_SVN) << "diff before repair:" << diff;
    QStringList lines = diff.split(QLatin1Char('\n'));
    QMap<QString, QString> headers;
    for(int a = 0; a < lines.size()-1; ++a) {
        const QLatin1String indexLineBegin("Index: ");
        if (lines.at(a).startsWith(indexLineBegin) && lines.at(a + 1).startsWith(QLatin1String("====="))) {
            const auto fileName = QStringView{lines.at(a)}.sliced(indexLineBegin.size()).trimmed().toString();
            headers[fileName] = lines.at(a);
            qCDebug(PLUGIN_SVN) << "found header for" << fileName;
            lines[a] = QString();
            if (lines.at(a + 1).startsWith(QLatin1String("======"))) {
                headers[fileName] += QLatin1Char('\n') + lines.at(a + 1);
                lines[a + 1] = QString();
            }
        }
    }

    static const QRegularExpression whitespaceRegex(QStringLiteral("\\s"));
    for(int a = 0; a < lines.size()-1; ++a) {
        const QLatin1String threeDashLineBegin("--- ");
        if (lines.at(a).startsWith(threeDashLineBegin)) {
            const auto tail = lines.at(a).mid(threeDashLineBegin.size());
            if (const auto whitespaceIndex = tail.indexOf(whitespaceRegex); whitespaceIndex != -1) {
                const auto file = tail.left(whitespaceIndex);
                qCDebug(PLUGIN_SVN) << "checking for" << file;
                const auto headerIt = headers.constFind(file);
                if (headerIt != headers.constEnd()) {
                    qCDebug(PLUGIN_SVN) << "adding header for" << file << ":" << *headerIt;
                    lines[a] = *headerIt + QLatin1Char('\n') + lines.at(a);
                }
            }
        }
    }
    QString ret = lines.join(QLatin1Char('\n'));
    qCDebug(PLUGIN_SVN) << "repaired diff:" << ret;
    return ret;
}

} // unnamed namespace

//@TODO: Handle raw diffs by using SvnCatJob to fetch both files/revisions

SvnInternalDiffJob::SvnInternalDiffJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
    m_pegRevision.setRevisionValue( KDevelop::VcsRevision::Head,
                                    KDevelop::VcsRevision::Special );
}

void SvnInternalDiffJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
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
                srcba = source().localUrl().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
            }else
            {
                srcba = source().repositoryServer().toUtf8();
            }
            QByteArray dstba;
            if( destination().type() == KDevelop::VcsLocation::LocalLocation )
            {
                dstba = destination().localUrl().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
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
                srcba = source().localUrl().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
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

    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while doing a diff: "
                << m_source.localUrl() << m_source.repositoryServer() << m_srcRevision.prettyValue()
                << m_destination.localUrl() << m_destination.repositoryServer() << m_dstRevision.prettyValue()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalDiffJob::setSource( const KDevelop::VcsLocation& src )
{
    QMutexLocker l( &m_mutex );
    m_source = src;
}
void SvnInternalDiffJob::setDestination( const KDevelop::VcsLocation& dst )
{
    QMutexLocker l( &m_mutex );
    m_destination = dst;
}
void SvnInternalDiffJob::setSrcRevision( const KDevelop::VcsRevision& srcRev )
{
    QMutexLocker l( &m_mutex );
    m_srcRevision = srcRev;
}
void SvnInternalDiffJob::setDstRevision( const KDevelop::VcsRevision& dstRev )
{
    QMutexLocker l( &m_mutex );
    m_dstRevision = dstRev;
}
void SvnInternalDiffJob::setPegRevision( const KDevelop::VcsRevision& pegRev )
{
    QMutexLocker l( &m_mutex );
    m_pegRevision = pegRev;
}
void SvnInternalDiffJob::setRecursive( bool recursive )
{
    QMutexLocker l( &m_mutex );
    m_recursive = recursive;
}
void SvnInternalDiffJob::setIgnoreAncestry( bool ignoreAncestry )
{
    QMutexLocker l( &m_mutex );
    m_ignoreAncestry = ignoreAncestry;
}
void SvnInternalDiffJob::setIgnoreContentType( bool ignoreContentType )
{
    QMutexLocker l( &m_mutex );
    m_ignoreContentType = ignoreContentType;
}
void SvnInternalDiffJob::setNoDiffOnDelete( bool noDiffOnDelete )
{
    QMutexLocker l( &m_mutex );
    m_noDiffOnDelete = noDiffOnDelete;
}

bool SvnInternalDiffJob::recursive() const
{
    QMutexLocker l( &m_mutex );
    return m_recursive;
}
bool SvnInternalDiffJob::ignoreAncestry() const
{
    QMutexLocker l( &m_mutex );
    return m_ignoreAncestry;
}
bool SvnInternalDiffJob::ignoreContentType() const
{
    QMutexLocker l( &m_mutex );
    return m_ignoreContentType;
}
bool SvnInternalDiffJob::noDiffOnDelete() const
{
    QMutexLocker l( &m_mutex );
    return m_noDiffOnDelete;
}
KDevelop::VcsLocation SvnInternalDiffJob::source() const
{
    QMutexLocker l( &m_mutex );
    return m_source;
}
KDevelop::VcsLocation SvnInternalDiffJob::destination() const
{
    QMutexLocker l( &m_mutex );
    return m_destination;
}
KDevelop::VcsRevision SvnInternalDiffJob::srcRevision() const
{
    QMutexLocker l( &m_mutex );
    return m_srcRevision;
}
KDevelop::VcsRevision SvnInternalDiffJob::dstRevision() const
{
    QMutexLocker l( &m_mutex );
    return m_dstRevision;
}
KDevelop::VcsRevision SvnInternalDiffJob::pegRevision() const
{
    QMutexLocker l( &m_mutex );
    return m_pegRevision;
}

SvnDiffJob::SvnDiffJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Add );
    connect( m_job.data(), &SvnInternalDiffJob::gotDiff,
                this, &SvnDiffJob::setDiff, Qt::QueuedConnection );

    setObjectName(i18n("Subversion Diff"));
}

QVariant SvnDiffJob::fetchResults()
{
    return QVariant::fromValue(m_diff);
}

void SvnDiffJob::start()
{
    if( !m_job->source().isValid()
         || ( !m_job->destination().isValid() &&
                ( m_job->srcRevision().revisionType() == KDevelop::VcsRevision::Invalid
                 || m_job->dstRevision().revisionType() == KDevelop::VcsRevision::Invalid ) )
      )
    {
        internalJobFailed();
        setErrorText( i18n( "Not enough information given to execute diff" ) );
    } else {
        startInternalJob();
    }
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
    m_diff.setBaseDiff(QUrl::fromLocalFile(QStringLiteral("/")));
    m_diff.setDiff( diff );

    emit resultsReady( this );
}

#include "moc_svndiffjob.cpp"
#include "moc_svndiffjob_p.cpp"
