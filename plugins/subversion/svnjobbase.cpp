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
#include <QStandardItemModel>

#include <kpassworddialog.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <kdebug.h>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iplugin.h>
#include <outputview/ioutputview.h>

#include "svninternaljobbase.h"
#include "svnssldialog.h"
#include "kdevsvnplugin.h"

SvnJobBase::SvnJobBase( KDevSvnPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity )
    : VcsJob( parent, verbosity ), m_part( parent ),
      m_status( KDevelop::VcsJob::JobNotStarted )
{
    setCapabilities( KJob::Killable );
    setTitle( "Subversion" );
}

SvnJobBase::~SvnJobBase()
{
}

bool SvnJobBase::doKill()
{
    internalJob()->kill();
    m_status = VcsJob::JobCanceled;
    return true;
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
    Q_UNUSED(path);
    outputMessage(msg);
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
    if ( m_status == VcsJob::JobFailed ) {
        // see: https://bugs.kde.org/show_bug.cgi?id=273759
        // this gets also called when the internal job failed
        // then the emit result in internalJobFailed might trigger
        // a nested event loop (i.e. error dialog)
        // during that the internalJobDone gets called and triggers
        // deleteLater and eventually deletes this job
        // => havoc
        // 
        // catching this state here works but I don't like it personally...
        return;
    }

    if( internalJob() == job )
    {
        outputMessage(i18n("Completed"));
        if( m_status != VcsJob::JobCanceled )
        {
            m_status = KDevelop::VcsJob::JobSucceeded;
        }
    }
    emitResult();
    if( m_status == VcsJob::JobCanceled )
    {
        deleteLater();
    }
}

void SvnJobBase::internalJobFailed( ThreadWeaver::Job* job )
{
    if( internalJob() == job )
    {
        setError( 255 );
        QString msg = internalJob()->errorMessage();
        if( !msg.isEmpty() )
            setErrorText( i18n( "Error executing Job:\n%1", msg ) );
        outputMessage(errorText());
        kDebug(9510) << "Job failed";
        if( m_status != VcsJob::JobCanceled )
        {
            m_status = KDevelop::VcsJob::JobFailed;
        }
        emitResult();
    }
    if( m_status == VcsJob::JobCanceled )
    {
        deleteLater();
    }
}

KDevelop::IPlugin* SvnJobBase::vcsPlugin() const
{
    return m_part;
}

void SvnJobBase::outputMessage(const QString& message)
{
    if (!model()) return;
    if (verbosity() == KDevelop::OutputJob::Silent) return;

    QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model());
    QStandardItem *previous = m->item(m->rowCount()-1);
    if (message == "." && previous && previous->text().contains(QRegExp("\\.+")))
        previous->setText(previous->text() + message);
    else
        m->appendRow(new QStandardItem(message));
    KDevelop::IPlugin* i = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            view->raiseOutput( outputId() );
        }
    }
}

#include "svnjobbase.moc"

