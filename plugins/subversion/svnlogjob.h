/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNLOGJOB_H
#define KDEVPLATFORM_PLUGIN_SVNLOGJOB_H

#include "svnjobbase.h"

#include <QVariant>

#include <QUrl>

#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>

class SvnInternalLogJob;

class SvnLogJob : public SvnJobBaseImpl<SvnInternalLogJob>
{
    Q_OBJECT
public:
    explicit SvnLogJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setLocation( const QUrl &location );
    void setLimit( int limit );
    void setEndRevision( const KDevelop::VcsRevision& rev );
    void setStartRevision( const KDevelop::VcsRevision& rev );
private Q_SLOTS:
    void logEventReceived( const KDevelop::VcsEvent& );
private:
    QList<QVariant> m_eventList;

};

#endif
