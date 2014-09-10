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
#include <KLocalizedString>

#include <ThreadWeaver.h>

#include "kdevsvncpp/client.hpp"
#include "kdevsvncpp/info.hpp"

SvnInternalInfoJob::SvnInternalInfoJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
}

void SvnInternalInfoJob::run()
{
    initBeforeRun();
    svn::Client cli(m_ctxt);
    try
    {
        QByteArray ba = location().toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        svn::InfoVector v = cli.info( ba.data() );
        svn::Info i = v.at(0);
        SvnInfoHolder h;
        h.name = QString::fromUtf8( i.path().path().c_str() );
        h.url = QUrl::fromUserInput( QString::fromUtf8( i.url() ) );
        h.rev = qlonglong( i.revision() );
        h.kind = i.kind();
        h.repoUrl = QUrl::fromUserInput( QString::fromUtf8( i.repos() ) );
        h.repouuid = QString::fromUtf8( i.uuid() );
        h.lastChangedRev = qlonglong( i.lastChangedRevision() );
        h.lastChangedDate = QDateTime::fromTime_t( i.lastChangedDate() );
        h.lastChangedAuthor = QString::fromUtf8( i.lastChangedAuthor() );
        h.scheduled = i.schedule();
        h.copyFromUrl = QUrl::fromUserInput( QString::fromUtf8( i.copyFromUrl() ) );
        h.copyFromRevision = qlonglong( i.copyFromRevision() );
        h.textTime = QDateTime::fromTime_t( i.textTime() );
        h.propertyTime = QDateTime::fromTime_t( i.propertyTime() );
        h.oldFileConflict = QString::fromUtf8( i.oldConflictFile() );
        h.newFileConflict = QString::fromUtf8( i.newConflictFile() );
        h.workingCopyFileConflict = QString::fromUtf8( i.workingConflictFile() );
        h.propertyRejectFile = QString::fromUtf8( i.propertyRejectFile() );

        emit gotInfo( h );
    }catch( svn::ClientException ce )
    {
        kDebug(9510) << "Exception while getting info for file: "
                << m_location
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}

void SvnInternalInfoJob::setLocation( const QUrl &url )
{
    QMutexLocker l( m_mutex );
    m_location = url;
}

QUrl SvnInternalInfoJob::location() const
{
    QMutexLocker l( m_mutex );
    return m_location;
}

SvnInfoJob::SvnInfoJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent ), m_provideInfo( SvnInfoJob::AllInfo )
{
    setType( KDevelop::VcsJob::Add );
    m_job = new SvnInternalInfoJob( this );
    connect( m_job, SIGNAL(gotInfo(SvnInfoHolder)),
             this, SLOT(setInfo(SvnInfoHolder)), Qt::QueuedConnection );
    setObjectName(i18n("Subversion Info"));
}

QVariant SvnInfoJob::fetchResults()
{
    if( m_provideInfo == RepoUrlOnly )
    {
        QUrl url = m_info.url;
        return qVariantFromValue<QUrl>( url );
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

void SvnInfoJob::setLocation( const QUrl &url )
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
