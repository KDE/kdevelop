/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNSTATUSJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNSTATUSJOB_P_H

#include "svninternaljobbase.h"

#include <vcs/vcsstatusinfo.h>
#include <QUrl>

class SvnInternalStatusJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalStatusJob( SvnJobBase* parent = nullptr );
    void setLocations( const QList<QUrl>& );
    void setRecursive( bool );

    bool recursive() const;
    QList<QUrl> locations() const;
Q_SIGNALS:
    void gotNewStatus( const KDevelop::VcsStatusInfo& );
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QList<QUrl> m_locations;
    bool m_recursive = false;
};

#endif

