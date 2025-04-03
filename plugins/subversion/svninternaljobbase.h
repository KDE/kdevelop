/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNINTERNALJOBBASE_H
#define KDEVPLATFORM_PLUGIN_SVNINTERNALJOBBASE_H

#include <ThreadWeaver/Job>

#include <QMutex>
#include <QObject>
#include <QSemaphore>

extern "C" {
#include <svn_wc.h>
}

#include "kdevsvncpp/context_listener.hpp"

namespace KDevelop
{
    class VcsRevision;
}

namespace svn
{
    class Context;
    class Revision;
}

class SvnJobBase;

class SvnInternalJobBase : public QObject, public ThreadWeaver::Job, public svn::ContextListener
{
    Q_OBJECT
public:
    explicit SvnInternalJobBase(SvnJobBase* parentJob);
    ~SvnInternalJobBase() override;

    bool success() const override;

    bool contextGetLogin( const std::string& realm,
                          std::string& username, std::string& password,
                          bool& maySave ) override;
    void contextNotify( const char* path, svn_wc_notify_action_t action,
                        svn_node_kind_t kind, const char* mimetype,
                        svn_wc_notify_state_t contentState,
                        svn_wc_notify_state_t propState, svn_revnum_t rev ) override;
    bool contextCancel() override;
    bool contextGetLogMessage( std::string& msg ) override;
    svn::ContextListener::SslServerTrustAnswer contextSslServerTrustPrompt(
            const svn::ContextListener::SslServerTrustData& data,
            apr_uint32_t& acceptedFailures ) override;
    bool contextSslClientCertPrompt( std::string& cert ) override;
    bool contextSslClientCertPwPrompt( std::string& pw, const std::string& realm,
                                       bool& maySave ) override;

    void initBeforeRun();
    
    void kill();
    
    bool wasKilled();

    QString errorMessage() const;

    svn::Context* m_ctxt;
    QSemaphore m_guiSemaphore;
    QString m_login_username;
    QString m_login_password;
    bool m_maySave;
    QString m_commitMessage;
    svn::ContextListener::SslServerTrustAnswer m_trustAnswer;

    static svn::Revision createSvnCppRevisionFromVcsRevision( const KDevelop::VcsRevision& );

Q_SIGNALS:
    void needLogin( const QString& );
    void showNotification( const QString&, const QString& );
    void needCommitMessage();
    void needSslServerTrust( const QStringList&, const QString&, const QString&,
                             const QString&, const QString&, const QString&,
                             const QString& );
    void needSslClientCert( const QString& );
    void needSslClientCertPassword( const QString& );

    /** This signal is emitted when this job is being processed by a thread. */
    void started();
    /**
     * This signal is emitted when the job finishes successfully (success() returns @c true).
     */
    void succeeded();
    /** This job has failed.
     *
     * This signal is emitted when success() returns false after the job is executed. */
    void failed();

protected:
    void defaultBegin(const ThreadWeaver::JobPointer& job, ThreadWeaver::Thread *thread) override;
    void defaultEnd(const ThreadWeaver::JobPointer& job, ThreadWeaver::Thread *thread) override;

    mutable QMutex m_mutex;
    mutable QMutex m_killMutex;
    bool m_success = true;
    void setErrorMessage( const QString& );

private:
    bool sendFirstDelta = false;
    bool killed = false;
    QString m_errorMessage;
    SvnJobBase* m_parentJob;
};


#endif

