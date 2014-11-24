/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
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

#ifndef KDEVPLATFORM_PLUGIN_SVNINTERNALJOBBASE_H
#define KDEVPLATFORM_PLUGIN_SVNINTERNALJOBBASE_H

#include <ThreadWeaver/Job>
#include <QEvent>
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
class QMutex;
class QSemaphore;
class SvnInternalJobBase : public QObject, public ThreadWeaver::Job, public svn::ContextListener
{
    Q_OBJECT
public:
    SvnInternalJobBase( SvnJobBase* parent = 0 );
    virtual ~SvnInternalJobBase();

    virtual bool success() const;

    bool contextGetLogin( const std::string& realm,
                          std::string& username, std::string& password,
                          bool& maySave );
    void contextNotify( const char* path, svn_wc_notify_action_t action,
                        svn_node_kind_t kind, const char* mimetype,
                        svn_wc_notify_state_t contentState,
                        svn_wc_notify_state_t propState, svn_revnum_t rev );
    bool contextCancel();
    bool contextGetLogMessage( std::string& msg );
    svn::ContextListener::SslServerTrustAnswer contextSslServerTrustPrompt(
            const svn::ContextListener::SslServerTrustData& data,
            apr_uint32_t& acceptedFailures );
    bool contextSslClientCertPrompt( std::string& cert );
    bool contextSslClientCertPwPrompt( std::string& pw, const std::string& realm,
                                       bool& maySave );

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

signals:
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
    /** This signal is emitted when the job has been finished (no matter if it succeeded or not). */
    void done();
    /** This job has failed.
     *
     * This signal is emitted when success() returns false after the job is executed. */
    void failed();

protected:
    void defaultBegin(const ThreadWeaver::JobPointer& job, ThreadWeaver::Thread *thread) Q_DECL_OVERRIDE;
    void defaultEnd(const ThreadWeaver::JobPointer& job, ThreadWeaver::Thread *thread) Q_DECL_OVERRIDE;

    QMutex* m_mutex;
    QMutex* m_killMutex;
    bool m_success;
    void setErrorMessage( const QString& );

private:
    bool sendFirstDelta;
    bool killed;
    QString m_errorMessage;
};


#endif

