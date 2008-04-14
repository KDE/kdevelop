/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVNJOBBASE_H
#define SVNJOBBASE_H

#include <vcs/vcsjob.h>

#include <kdevsvncpp/context_listener.hpp>

extern "C"
{
#include <svn_wc.h>
}


class QEvent;
class SvnInternalJobBase;

namespace ThreadWeaver
{
    class Job;
}

class KDevSvnPlugin;

class SvnJobBase : public KDevelop::VcsJob
{
    Q_OBJECT
public:
    SvnJobBase( KDevSvnPlugin* );
    virtual ~SvnJobBase();
    virtual SvnInternalJobBase* internalJob() const = 0;
    KDevelop::VcsJob::JobStatus status() const;
    KDevelop::IPlugin* vcsPlugin() const;
public slots:
    void askForLogin( const QString& );
    void showNotification( const QString&, const QString& );
    void askForCommitMessage();
    void askForSslServerTrust( const QStringList&, const QString&, const QString&,
                               const QString&, const QString&, const QString&,
                               const QString& );
    void askForSslClientCert( const QString& );
    void askForSslClientCertPassword( const QString& );

protected slots:
    void internalJobStarted( ThreadWeaver::Job* job );
    void internalJobDone( ThreadWeaver::Job* job );
    void internalJobFailed( ThreadWeaver::Job* job );

protected:
    KDevSvnPlugin* m_part;

private:
    KDevelop::VcsJob::JobStatus m_status;
};

#endif
