/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNSTATUSJOB_H
#define KDEVPLATFORM_PLUGIN_SVNSTATUSJOB_H

#include "svnjobbase.h"

#include <vcs/vcsstatusinfo.h>
#include <QUrl>

class SvnInternalStatusJob;

class SvnStatusJob : public SvnJobBaseImpl<SvnInternalStatusJob>
{
    Q_OBJECT
public:

    explicit SvnStatusJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;

    void setLocations( const QList<QUrl>& locations );
    void setRecursive( bool );

public Q_SLOTS:
    void addToStats( const KDevelop::VcsStatusInfo& );
private:
    QList<QVariant> m_stats;
};

#endif

