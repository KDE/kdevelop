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

#include "svncatjob.h"
#include "svncatjob_p.h"

#include <QMutexLocker>

#include <KLocalizedString>
#include <ThreadWeaver.h>

#include <vcs/vcsrevision.h>

#include "kdevsvncpp/path.hpp"
#include "kdevsvncpp/revision.hpp"

#include "svnclient.h"

SvnInternalCatJob::SvnInternalCatJob( SvnJobBase* parent )
    : SvnInternalJobBase( parent )
{
    m_pegRevision.setRevisionValue( KDevelop::VcsRevision::Head,
                                    KDevelop::VcsRevision::Special );
}

void SvnInternalCatJob::run()
{
    initBeforeRun();

    SvnClient cli(m_ctxt);
    try
    {

        QString content;
        QByteArray srcba;
        if( source().type() == KDevelop::VcsLocation::LocalLocation )
        {
            QUrl url = source().localUrl();
            srcba = url.toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ).toUtf8();
        }else
        {
            srcba = source().repositoryServer().toUtf8();
        }
        svn::Revision srcRev = createSvnCppRevisionFromVcsRevision( srcRevision() );
        content = QString::fromUtf8( cli.cat( svn::Path( srcba.data() ), srcRev ).c_str() );
        emit gotContent( content );
    }catch( svn::ClientException ce )
    {
        qCDebug(PLUGIN_SVN) << "Exception while doing a diff: "
                << m_source.localUrl() << m_source.repositoryServer() << m_srcRevision.revisionValue()
                << QString::fromUtf8( ce.message() );
        setErrorMessage( QString::fromUtf8( ce.message() ) );
        m_success = false;
    }
}


void SvnInternalCatJob::setSource( const KDevelop::VcsLocation& src )
{
    QMutexLocker l( m_mutex );
    m_source = src;
}
void SvnInternalCatJob::setSrcRevision( const KDevelop::VcsRevision& srcRev )
{
    QMutexLocker l( m_mutex );
    m_srcRevision = srcRev;
}
void SvnInternalCatJob::setPegRevision( const KDevelop::VcsRevision& pegRev )
{
    QMutexLocker l( m_mutex );
    m_pegRevision = pegRev;
}
KDevelop::VcsLocation SvnInternalCatJob::source() const
{
    QMutexLocker l( m_mutex );
    return m_source;
}
KDevelop::VcsRevision SvnInternalCatJob::srcRevision() const
{
    QMutexLocker l( m_mutex );
    return m_srcRevision;
}
KDevelop::VcsRevision SvnInternalCatJob::pegRevision() const
{
    QMutexLocker l( m_mutex );
    return m_pegRevision;
}

SvnCatJob::SvnCatJob( KDevSvnPlugin* parent )
    : SvnJobBase( parent, KDevelop::OutputJob::Silent )
{
    setType( KDevelop::VcsJob::Cat );
    m_job = new SvnInternalCatJob( this );
    setObjectName(i18n("Subversion Cat"));
}

QVariant SvnCatJob::fetchResults()
{
    return m_content;
}

void SvnCatJob::start()
{
    if( !m_job->source().isValid() )
    {
        internalJobFailed( m_job );
        setErrorText( i18n( "Not enough information to execute cat" ) );
    }else
    {
        connect( m_job, SIGNAL(gotContent(QString)),
                 this, SLOT(setContent(QString)),
                 Qt::QueuedConnection );
        ThreadWeaver::Weaver::instance()->enqueue( m_job );
    }
}

SvnInternalJobBase* SvnCatJob::internalJob() const
{
    return m_job;
}

void SvnCatJob::setSource( const KDevelop::VcsLocation& source )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setSource( source );
}
void SvnCatJob::setPegRevision( const KDevelop::VcsRevision& pegRevision )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setPegRevision( pegRevision );
}

void SvnCatJob::setSrcRevision( const KDevelop::VcsRevision& srcRevision )
{
    if( status() == KDevelop::VcsJob::JobNotStarted )
        m_job->setSrcRevision( srcRevision );
}

void SvnCatJob::setContent( const QString& content )
{
    m_content = content;
    emit resultsReady( this );
}
