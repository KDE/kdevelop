/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNUPDATEJOB_H
#define KDEVPLATFORM_PLUGIN_SVNUPDATEJOB_H

#include "svnjobbase.h"

#include <vcs/vcsrevision.h>
#include <QUrl>

class SvnInternalUpdateJob;

class SvnUpdateJob : public SvnJobBaseImpl<SvnInternalUpdateJob>
{
    Q_OBJECT
public:
    explicit SvnUpdateJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;

    void setLocations( const QList<QUrl>& locations );
    void setRecursive( bool );
    void setRevision( const KDevelop::VcsRevision& );
    void setIgnoreExternals( bool );
};

#endif

