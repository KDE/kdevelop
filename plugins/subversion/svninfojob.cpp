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

#include "svninfojob.h"

#include "svninfojob_p.h"

#include <QMutexLocker>

#include <kdebug.h>
#include <klocale.h>

#include <ThreadWeaver.h>

#include <svncpp/client.hpp>
#include <svncpp/entry.hpp>

SvnInternalInfoJob::SvnInternalInfoJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalInfoJob::run()
{
    initBeforeRun();
    SvnInfoJobHelper help;
    connect( &help, SIGNAL( gotInfo( const SvnInfoHolder&) ),
              this, SIGNAL( gotInfo( const SvnInfoHolder&) ) );
    svn::Client cli(m_ctxt);
    try
    {
        QByteArray ba = location().path().toUtf8();
        svn::Entry e = cli.info( ba.data() );
        SvnInfoHolder h;
        h.name = QString::fromUtf8( e.name() );
        h.url = KUrl( QString::fromUtf8( e.url() ) );
        h.rev = qlonglong( e.revision() );
        h.kind = e.kind();
        h.repoUrl = KUrl( QString::fromUtf8( e.repos() ) );
        h.repouuid = QString::fromUtf8( e.uuid() );
        h.lastChangedRev = qlonglong( e.cmtRev() );
        h.lastChangedDate = QDateTime::fromTime_t( e.cmtDate() );
        h.lastChangedAuthor = QString::fromUtf8( e.cmtAuthor() );
        h.scheduled = e.schedule();
        h.copyFromUrl = KUrl( QString::fromUtf8( e.copyfromUrl() ) );
        h.copyFromRevision = qlonglong( e.copyfromRev() );
        h.textTime = QDateTime::fromTime_t( e.textTime() );
        h.propertyTime = QDateTime::fromTime_t( e.propTime() );
        h.oldFileConflict = QString::fromUtf8( e.conflictOld() );
        h.newFileConflict = QString::fromUtf8( e.conflictNew() );
        h.workingCopyFileConflict = QString::fromUtf8( e.conflictWrk() );
        h.propertyRejectFile = QString::fromUtf8( e.prejfile() );

        help.emitInfo( h );
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while getting info for file: "
                << m_location
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalInfoJob::setLocation( const KUrl& url )
{
    QMutexLocker l( m_mutex );
    m_location = url;
}

KUrl SvnInternalInfoJob::location() const
{
    QMutexLocker l( m_mutex );
    return m_location;
}

SvnInfoJob::SvnInfoJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent ), m_provideInfo( SvnInfoJob::AllInfo )
{
    setType( KDevelop::VcsJob::Add );
    m_job = new SvnInternalInfoJob( this );
    connect( m_job, SIGNAL( gotInfo( const SvnInfoHolder& ) ),
             this, SLOT( setInfo( const SvnInfoHolder& ) ), Qt::QueuedConnection );
}

QVariant SvnInfoJob::fetchResults()
{
    if( m_provideInfo == RepoUrlOnly )
    {
        KUrl url = m_info.url;
        return qVariantFromValue<KUrl>( url );
    }else if( m_provideInfo == RevisionOnly )
    {
        KDevelop::VcsRevision rev;
        svn::Revision svnRev( m_info.rev );
        switch( m_provideRevisionType )
        {
            case KDevelop::VcsRevision::Date:
                rev.setRevisionValue( QVariant( QDateTime::fromTime_t(  svnRev.date() ) ),
                                      KDevelop::VcsRevision::Date );
                break;
            default:
                rev.setRevisionValue( QVariant( qlonglong( svnRev.revnum() ) ),
                                      KDevelop::VcsRevision::GlobalNumber );
                break;
        }
        return qVariantFromValue<KDevelop::VcsRevision>( rev );
    }
    return qVariantFromValue<SvnInfoHolder>( m_info );
}

void SvnInfoJob::start()
{
    if( !m_job->location().isValid() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to execute info job" ) );
    }else
    {
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnInfoJob::internalJob() const
{
    return m_job;
}

void SvnInfoJob::setLocation( const KUrl& url )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setLocation( url );
}

void SvnInfoJob::setProvideInformation( ProvideInformationType type )
{
    m_provideInfo = type;
}

void SvnInfoJob::setProvideRevisionType( KDevelop::VcsRevision::RevisionType t )
{
    m_provideRevisionType = t;
}

void SvnInfoJob::setInfo( const SvnInfoHolder& info )
{
    m_info = info;
    emit resultsReady( this );
}

#include "svninfojob.moc"
#include "svninfojob_p.moc"

