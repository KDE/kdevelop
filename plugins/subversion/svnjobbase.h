/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNJOBBASE_H
#define KDEVPLATFORM_PLUGIN_SVNJOBBASE_H

#include <vcs/vcsjob.h>

#include "kdevsvncpp/context_listener.hpp"
#include "kdevsvnplugin.h"
#include "debug.h"

#include <ThreadWeaver/Queueing>
#include <QSharedPointer>

extern "C"
{
#include <svn_wc.h>
}

class SvnInternalJobBase;
using SvnInternalJobBasePtr = QSharedPointer<SvnInternalJobBase>;

namespace ThreadWeaver
{
    class Job;
}

class KDevSvnPlugin;

class SvnJobBase : public KDevelop::VcsJob
{
    Q_OBJECT
public:
    enum {
        // Add a "random" number to KJob::UserDefinedError and hopefully avoid
        // clashes with OutputJob's, OutputExecuteJob's, etc. error codes.
        FailedToStart = UserDefinedError + 151,
        InternalJobFailed,
    };

    explicit SvnJobBase( KDevSvnPlugin*, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose );
    ~SvnJobBase() override;
    virtual SvnInternalJobBasePtr internalJob() const = 0;
    KDevelop::VcsJob::JobStatus status() const override;
    KDevelop::IPlugin* vcsPlugin() const override;

public Q_SLOTS:
    void askForLogin( const QString& );
    void showNotification( const QString&, const QString& );
    void askForCommitMessage();
    void askForSslServerTrust( const QStringList&, const QString&, const QString&,
                               const QString&, const QString&, const QString&,
                               const QString& );
    void askForSslClientCert( const QString& );
    void askForSslClientCertPassword( const QString& );

protected:
    /**
     * Call this function instead of startInternalJob() if an error is detected in start().
     *
     * @param errorMessage a nonempty translatable error message that tells the user why the job has failed
     */
    void failToStart(const QString& errorMessage);
    /**
     * Call this function from start() in order to run the internal job.
     *
     * If verbosity() is Verbose, this function calls startOutput() before starting the internal job.
     *
     * @param introductoryOutputMessage a message to output if it is not empty and verbosity() is Verbose
     */
    void startInternalJob(const QString& introductoryOutputMessage = {});

    bool doKill() override;

private:
    void internalJobStarted();
    void internalJobSucceeded();
    void internalJobFailed();

    /**
     * Update the status of this job and emit result.
     *
     * @pre the properties error and errorText inherited from KJob have already been set
     */
    void failJob();

    void outputMessage(const QString &message);

    KDevSvnPlugin* const m_part;
    VcsJob::JobStatus m_status = VcsJob::JobNotStarted;
};

template<typename InternalJobClass>
class SvnJobBaseImpl : public SvnJobBase
{
public:
    explicit SvnJobBaseImpl(KDevSvnPlugin* plugin,
                   KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose)
        : SvnJobBase(plugin, verbosity)
        , m_job(new InternalJobClass(this))
    {
    }

    SvnInternalJobBasePtr internalJob() const override
    {
        return m_job;
    }

protected:
    QSharedPointer<InternalJobClass> m_job;
};

#endif
