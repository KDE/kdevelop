/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "svnjobbase.h"

#include <QStandardItemModel>

#include <KPasswordDialog>
#include <KLocalizedString>
#include <KMessageBox>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iplugin.h>
#include <outputview/ioutputview.h>

#include "svninternaljobbase.h"
#include "svnssldialog.h"

#include <algorithm>

SvnJobBase::SvnJobBase( KDevSvnPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity )
    // Pass Silent to VcsJob() in order to prevent it from invoking startOutput()
    // asynchronously. this->startInternalJob() calls startOutput() for a Verbose
    // job synchronously so as to be able to produce output immediately.
    : VcsJob(parent, OutputJob::Silent)
    , m_part(parent)
{
    // Now manually set verbosity to the requested value.
    setVerbosity(verbosity);
    setCapabilities( KJob::Killable );
    setTitle( QStringLiteral("Subversion") );
}

SvnJobBase::~SvnJobBase()
{
}

void SvnJobBase::failToStart(const QString& errorMessage)
{
    qCDebug(PLUGIN_SVN) << "job failed to start" << this;
    Q_ASSERT(!errorMessage.isEmpty());

    setError(FailedToStart);
    setErrorText(errorMessage);
    failJob();
}

void SvnJobBase::startInternalJob(const QString& introductoryOutputMessage)
{
    if (verbosity() == OutputJob::Verbose) {
        startOutput();
        if (!introductoryOutputMessage.isEmpty()) {
            outputMessage(introductoryOutputMessage);
        }
    }

    auto job = internalJob();
    connect( job.data(), &SvnInternalJobBase::failed,
             this, &SvnJobBase::internalJobFailed, Qt::QueuedConnection );
    connect( job.data(), &SvnInternalJobBase::done,
             this, &SvnJobBase::internalJobDone, Qt::QueuedConnection );
    connect( job.data(), &SvnInternalJobBase::started,
             this, &SvnJobBase::internalJobStarted, Qt::QueuedConnection );
    // add as shared pointer
    // the signals "done" & "failed" are emitted when the queue and the executor still
    // have and use a reference to the job, in the execution thread.
    // As the this parent job will be deleted in the main/other thread
    // (due to deleteLater() being called on it in the KJob::exec())
    // and the ThreadWeaver queue will release the last reference to the passed
    // JobInterface pointer only after the JobInterface::execute() method has been left,
    // the internal threaded job thus needs to get shared memory management via the QSharedPointer.
    m_part->jobQueue()->stream() << job;
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
    qCDebug(PLUGIN_SVN) << "login";
    KPasswordDialog dlg( nullptr, KPasswordDialog::ShowUsernameLine | KPasswordDialog::ShowKeepPassword );
    dlg.setPrompt( i18n("Enter Login for: %1", realm ) );
    if (dlg.exec()) { // krazy:exclude=crashy
        internalJob()->m_login_username = dlg.username();
        internalJob()->m_login_password = dlg.password();
        internalJob()->m_maySave = dlg.keepPassword();
    } else {
        internalJob()->m_login_username.clear();
        internalJob()->m_login_password.clear();
    }
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::showNotification( const QString& path, const QString& msg )
{
    Q_UNUSED(path);
    outputMessage(msg);
}

void SvnJobBase::askForCommitMessage()
{
    qCDebug(PLUGIN_SVN) << "commit msg";
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::askForSslServerTrust( const QStringList& failures, const QString& host,
                                       const QString& print, const QString& from,
                                       const QString& until, const QString& issuer,
                                       const QString& realm )
{

    qCDebug(PLUGIN_SVN) << "servertrust";
    SvnSSLTrustDialog dlg;
    dlg.setCertInfos( host, print, from, until, issuer, realm, failures );
    if( dlg.exec() == QDialog::Accepted )
    {
        qCDebug(PLUGIN_SVN) << "accepted with:" << dlg.useTemporarily();
        if( dlg.useTemporarily() )
        {
            internalJob()->m_trustAnswer = svn::ContextListener::ACCEPT_TEMPORARILY;
        }else
        {
        internalJob()->m_trustAnswer = svn::ContextListener::ACCEPT_PERMANENTLY;
        }
    }else
    {
        qCDebug(PLUGIN_SVN) << "didn't accept";
        internalJob()->m_trustAnswer = svn::ContextListener::DONT_ACCEPT;
    }
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::askForSslClientCert( const QString& realm )
{
    KMessageBox::information( nullptr, realm );
    qCDebug(PLUGIN_SVN) << "clientrust";
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::askForSslClientCertPassword( const QString& )
{
    qCDebug(PLUGIN_SVN) << "clientpw";
    internalJob()->m_guiSemaphore.release( 1 );
}

void SvnJobBase::internalJobStarted()
{
    qCDebug(PLUGIN_SVN) << "job started" << this;
    m_status = KDevelop::VcsJob::JobRunning;
}

void SvnJobBase::internalJobDone()
{
    qCDebug(PLUGIN_SVN) << "job done" << this;
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

    outputMessage(i18n("Completed"));
    if( m_status != VcsJob::JobCanceled ) {
        m_status = KDevelop::VcsJob::JobSucceeded;
    }

    emitResult();
}

void SvnJobBase::internalJobFailed()
{
    qCDebug(PLUGIN_SVN) << "job failed" << this;

    setError(InternalJobFailed);
    QString msg = internalJob()->errorMessage();
    if( !msg.isEmpty() )
        setErrorText( i18n( "Error executing Job:\n%1", msg ) );
    outputMessage(errorText());
    failJob();
}

void SvnJobBase::failJob()
{
    if( m_status != VcsJob::JobCanceled )
    {
        m_status = KDevelop::VcsJob::JobFailed;
    }
    emitResult();
}

KDevelop::IPlugin* SvnJobBase::vcsPlugin() const
{
    return m_part;
}

void SvnJobBase::outputMessage(const QString& message)
{
    if (verbosity() == KDevelop::OutputJob::Silent) return;

    static constexpr QLatin1Char dot{'.'};
    const auto consistsOfDots = [](const QString& text) {
        return std::all_of(text.cbegin(), text.cend(), [](QChar c) {
            return c == dot;
        });
    };

    auto *m = qobject_cast<QStandardItemModel*>(model());
    Q_ASSERT(m);
    // Be smart when showing the svn output. When it says "." on committing each file,
    // append it to the "." on the previous line (just like svn command does).
    QStandardItem *previous = m->item(m->rowCount()-1);
    if (QString previousText; message == dot && previous && consistsOfDots(previousText = previous->text())) {
        previous->setText(previousText += dot);
    } else {
        m->appendRow(new QStandardItem(message));
    }

    KDevelop::IPlugin* i = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IOutputView"));
    if( i )
    {
        auto* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            view->raiseOutput( outputId() );
        }
    }
}

#include "moc_svnjobbase.cpp"
