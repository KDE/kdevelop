/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svnblamejob.h"

#include "svnblamejob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>

#include "svnclient.h"
#include <QDateTime>

SvnInternalBlameJob::SvnInternalBlameJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
    m_startRevision.setRevisionValue(QVariant::fromValue(KDevelop::VcsRevision::Start),
                                    KDevelop::VcsRevision::Special );
    m_endRevision.setRevisionValue(QVariant::fromValue(KDevelop::VcsRevision::Head),
                                    KDevelop::VcsRevision::Special );
}

void SvnInternalBlameJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    initBeforeRun();

    QByteArray ba = location().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();

    svn::Client cli(m_ctxt);
    svn::AnnotatedFile* file;
    try
    {
        file = cli.annotate( ba.data(),
                             createSvnCppRevisionFromVcsRevision( startRevision() ),
                             createSvnCppRevisionFromVcsRevision( endRevision() ) );
    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while blaming file: "
                << location()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
        return;
    }
    svn_revnum_t minrev = -1, maxrev = -1;
    for (const auto& line : *file) {
        const svn_revnum_t lineRevision = line.revision();
        if (lineRevision < minrev || minrev == -1) {
            minrev = lineRevision;
        }
        if (lineRevision > maxrev || maxrev == -1 ) {
            maxrev = lineRevision;
        }
    }
    QHash<svn_revnum_t,QString> commitMessages;
    try
    {
        const svn::LogEntries* entries = cli.log( ba.data(), svn::Revision(minrev), svn::Revision(maxrev), false, false );
        for (const auto& entry : *entries) {
            commitMessages[entry.revision] = QString::fromUtf8(entry.message.c_str() );
        }
    }catch( const svn::ClientException& ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while fetching log messages for blame: "
                     << location()
                     << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
    for (const auto& svnLine : *file) {
        KDevelop::VcsAnnotationLine line;
        line.setAuthor(QString::fromUtf8(svnLine.author().c_str()));
        line.setDate(QDateTime::fromString(QString::fromUtf8(svnLine.date().c_str()), Qt::ISODate));
        line.setText(QString::fromUtf8(svnLine.line().c_str()));
        KDevelop::VcsRevision rev;
        rev.setRevisionValue(QVariant(qlonglong(svnLine.revision())), KDevelop::VcsRevision::GlobalNumber);
        line.setRevision( rev );
        line.setLineNumber(svnLine.lineNumber());
        line.setCommitMessage(commitMessages[svnLine.revision()]);
        emit blameLine( line );
    }
}

void SvnInternalBlameJob::setLocation( const QUrl &url )
{
    QMutexLocker l( &m_mutex );
    m_location = url;
}

QUrl SvnInternalBlameJob::location() const
{
    QMutexLocker l( &m_mutex );
    return m_location;
}

KDevelop::VcsRevision SvnInternalBlameJob::startRevision() const
{
    QMutexLocker l( &m_mutex );
    return m_startRevision;
}

KDevelop::VcsRevision SvnInternalBlameJob::endRevision() const
{
    QMutexLocker l( &m_mutex );
    return m_endRevision;
}

void SvnInternalBlameJob::setStartRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( &m_mutex );
    m_startRevision = rev;
}

void SvnInternalBlameJob::setEndRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( &m_mutex );
    m_endRevision = rev;
}

SvnBlameJob::SvnBlameJob( KDevSvnPlugin* parent )
    : SvnJobBaseImpl( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Annotate );
    connect(m_job.data(), &SvnInternalBlameJob::blameLine,
            this, &SvnBlameJob::blameLineReceived);
    setObjectName(i18n("Subversion Annotate"));
}

QVariant SvnBlameJob::fetchResults()
{
    QList<QVariant> results = m_annotations;
    m_annotations.clear();
    return results;
}

void SvnBlameJob::start()
{
    if ( !m_job->location().isValid() ) {
        failToStart(i18n("Not enough information to blame location"));
    } else {
        qCDebug(PLUGIN_SVN) << "blaming url:" << m_job->location();
        startInternalJob();
    }
}

void SvnBlameJob::setLocation( const QUrl &url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocation( url );
}

void SvnBlameJob::setStartRevision( const KDevelop::VcsRevision& rev )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setStartRevision( rev );
}

void SvnBlameJob::setEndRevision( const KDevelop::VcsRevision& rev )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setEndRevision( rev );
}

void SvnBlameJob::blameLineReceived( const KDevelop::VcsAnnotationLine& line )
{
    m_annotations.append(QVariant::fromValue(line));
    emit resultsReady( this );
}

#include "moc_svnblamejob_p.cpp"
#include "moc_svnblamejob.cpp"
