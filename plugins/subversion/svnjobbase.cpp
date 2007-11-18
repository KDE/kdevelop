/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnjobbase.h"

#include <QEvent>

#include <kpassworddialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "svninternaljobbase.h"
#include "svnssldialog.h"
#include "kdevsvnplugin.h"
#include "svnoutputmodel.h"

SvnJobBase::SvnJobBase( KDevSvnPlugin* parent )
    : VcsJob( parent ), m_part( parent ),
      m_status( KDevelop::VcsJob::JobNotStarted )
{
}

SvnJobBase::~SvnJobBase()
{
}

KDevelop::VcsJob::JobStatus SvnJobBase::status() const
{
    return m_status;
}


void SvnJobBase::askForLogin( const QString& realm )
{
    kDebug( 9510 ) << "login";
    KPasswordDialog dlg( 0, KPasswordDialog::ShowUsernameLine | KPasswordDialog::ShowKeepPassword );
    dlg.setPrompt( i18n("Enter Login for: %1", realm ) );
    dlg.exec();
    internalJob()->m_login_username = dlg.username();
    internalJob()->m_login_password = dlg.password();
    internalJob()->m_maySave = dlg.keepPassword();
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::showNotification( const QString& path, const QString& msg )
{
    kDebug( 9510 ) << "notification" << path << msg;
    m_part->outputModel()->appendRow( new SvnOutputItem( path, msg ) );
}

void SvnJobBase::askForCommitMessage()
{
    kDebug( 9510 ) << "commit msg";
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::askForSslServerTrust( const QStringList& failures, const QString& host,
                                       const QString& print, const QString& from,
                                       const QString& until, const QString& issuer,
                                       const QString& realm )
{

    kDebug( 9510 ) << "servertrust";
    SvnSSLTrustDialog dlg;
    dlg.setCertInfos( host, print, from, until, issuer, realm, failures );
    if( dlg.exec() == QDialog::Accepted )
    {
        kDebug(9510) << "accepted with:" << dlg.useTemporarily();
        if( dlg.useTemporarily() )
        {
            internalJob()->m_trustAnswer = svn::ContextListener::ACCEPT_TEMPORARILY;
        }else
        {
        internalJob()->m_trustAnswer = svn::ContextListener::ACCEPT_PERMANENTLY;
        }
    }else
    {
        kDebug(9510) << "didn't accept";
        internalJob()->m_trustAnswer = svn::ContextListener::DONT_ACCEPT;
    }
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::askForSslClientCert( const QString& realm )
{
    KMessageBox::information( 0, realm );
    kDebug( 9510 ) << "clientrust";
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::askForSslClientCertPassword( const QString& )
{
    kDebug( 9510 ) << "clientpw";
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::internalJobStarted( ThreadWeaver::Job* job )
{
    if( internalJob() == job )
    {
        m_status = KDevelop::VcsJob::JobRunning;
    }
}

void SvnJobBase::internalJobDone( ThreadWeaver::Job* job )
{
    if( internalJob() == job )
    {
        kDebug(9510) << "Job is done";
        m_status = KDevelop::VcsJob::JobSucceeded;
    }
    emitResult();
}

void SvnJobBase::internalJobFailed( ThreadWeaver::Job* job )
{
    if( internalJob() == job )
    {
        setError( 255 );
        QString msg = internalJob()->errorMessage();
        if( !msg.isEmpty() )
            setErrorText( i18n( "Error executing Job:\n%1", msg ) );
        kDebug(9510) << "Job failed";
        m_status = KDevelop::VcsJob::JobFailed;
    }
    emitResult();
}

KDevelop::IPlugin* SvnJobBase::vcsPlugin() const
{
    return m_part;
}

#include "svnjobbase.moc"

