/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNCOMMITJOB_H
#define KDEVPLATFORM_PLUGIN_SVNCOMMITJOB_H

#include "svnjobbase.h"
#include <QUrl>
#include <ThreadWeaver/Job>

class SvnInternalCommitJob;
class SvnCommitDialog;

class SvnCommitJob : public SvnJobBaseImpl<SvnInternalCommitJob>
{
    Q_OBJECT
public:
    explicit SvnCommitJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setCommitMessage( const QString& msg );
    void setKeepLock( bool );
    void setUrls( const QList<QUrl>& urls );
    void setRecursive( bool );
};

#endif

