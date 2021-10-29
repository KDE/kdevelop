/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNADDJOB_H
#define KDEVPLATFORM_PLUGIN_SVNADDJOB_H

#include "svnjobbase.h"

#include <QUrl>

class SvnInternalAddJob;

class SvnAddJob : public SvnJobBaseImpl<SvnInternalAddJob>
{
    Q_OBJECT
public:
    explicit SvnAddJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;

    void setLocations( const QList<QUrl>& locations );
    void setRecursive( bool );
};

#endif

