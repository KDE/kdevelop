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

#include "svnblamejob.h"

#include "svnblamejob_p.h"

#include <QMutexLocker>

#include <klocale.h>
#include <kdebug.h>
#include <ThreadWeaver.h>

#include "svnclient.h"
#include <QDateTime>

SvnInternalBlameJob::SvnInternalBlameJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
    m_startRevision.setRevisionValue( qVariantFromValue( KDevelop::VcsRevision::Start ),
                                    KDevelop::VcsRevision::Special );
    m_endRevision.setRevisionValue( qVariantFromValue( KDevelop::VcsRevision::Head ),
                                    KDevelop::VcsRevision::Special );
}

void SvnInternalBlameJob::run()
{
    initBeforeRun();

    QByteArray ba = location().toLocalFile( KUrl::RemoveTrailingSlash ).toUtf8();
    
    svn::Client cli(m_ctxt);
    svn::AnnotatedFile* file;
    try
    {
        file = cli.annotate( ba.data(),
                             createSvnCppRevisionFromVcsRevision( startRevision() ),
                             createSvnCppRevisionFromVcsRevision( endRevision() ) );
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while blaming file: "
                << location()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
        return;
    }
    svn_revnum_t minrev = -1, maxrev = -1;
    for( svn::AnnotatedFile::const_iterator it = file->begin(); it != file->end(); it++ )
    {
        if( (*it).revision() < minrev || minrev == -1 )
        {
            minrev = (*it).revision();
        }
        if( (*it).revision() > maxrev || maxrev == -1 )
        {
            maxrev = (*it).revision();
        }
    }
    QHash<svn_revnum_t,QString> commitMessages;
    try
    {
        const svn::LogEntries* entries = cli.log( ba.data(), svn::Revision(minrev), svn::Revision(maxrev), false, false );
        for( svn::LogEntries::const_iterator it = entries->begin(); it != entries->end(); it++ )
        {
            commitMessages[(*it).revision] = QString::fromUtf8( (*it).message.c_str() );
        }
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while fetching log messages for blame: "
                     << location()
                     << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
    for( svn::AnnotatedFile::const_iterator it = file->begin(); it != file->end(); it++ )
    {
        KDevelop::VcsAnnotationLine line;
        line.setAuthor( QString::fromUtf8( it->author().c_str() ) );
        line.setDate( QDateTime::fromString( QString::fromUtf8( it->date().c_str() ), Qt::ISODate ) );
        line.setText( QString::fromUtf8( it->line().c_str() ) );
        KDevelop::VcsRevision rev;
        rev.setRevisionValue( QVariant( qlonglong( it->revision() ) ), KDevelop::VcsRevision::GlobalNumber );
        line.setRevision( rev );
        line.setLineNumber( it->lineNumber() );
        line.setCommitMessage( commitMessages[it->revision()] );
        emit blameLine( line );
    }
}

void SvnInternalBlameJob::setLocation( const KUrl& url )
{
    QMutexLocker l( m_mutex );
    m_location = url;
}

KUrl SvnInternalBlameJob::location() const
{
    QMutexLocker l( m_mutex );
    return m_location;
}

KDevelop::VcsRevision SvnInternalBlameJob::startRevision() const
{
    QMutexLocker l( m_mutex );
    return m_startRevision;
}

KDevelop::VcsRevision SvnInternalBlameJob::endRevision() const
{
    QMutexLocker l( m_mutex );
    return m_endRevision;
}

void SvnInternalBlameJob::setStartRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( m_mutex );
    m_startRevision = rev;
}

void SvnInternalBlameJob::setEndRevision( const KDevelop::VcsRevision& rev )
{
    QMutexLocker l( m_mutex );
    m_endRevision = rev;
}

SvnBlameJob::SvnBlameJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Annotate );
    m_job = new SvnInternalBlameJob( this );
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
    if( !m_job->location().isValid() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to blame location" ) );
    }else
    {
        connect( m_job, SIGNAL(blameLine(KDevelop::VcsAnnotationLine)),
                 this, SLOT(blameLineReceived(KDevelop::VcsAnnotationLine)) );
        kDebug(9510) << "blameging url:" << m_job->location();
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnBlameJob::internalJob() const
{
    return m_job;
}

void SvnBlameJob::setLocation( const KUrl& url )
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
    m_annotations.append( qVariantFromValue( line ) );
    emit resultsReady( this );
}
