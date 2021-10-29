/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNCOMMITJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNCOMMITJOB_P_H

#include <QUrl>
#include "svninternaljobbase.h"

class SvnJobBase;

class SvnInternalCommitJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalCommitJob( SvnJobBase* parent = nullptr );
    void setRecursive( bool );
    void setCommitMessage( const QString& );
    void setUrls( const QList<QUrl>& );
    void setKeepLock( bool );

    QList<QUrl> urls() const;
    QString commitMessage() const;
    bool recursive() const;
    bool keepLock() const;

protected:
    void run(ThreadWeaver::JobPointer job, ThreadWeaver::Thread* thread) override;
private:
    QList<QUrl> m_urls;
    bool m_recursive = true;
    bool m_keepLock = false;
};


#endif

