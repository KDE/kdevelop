/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNREVERTJOB_H
#define KDEVPLATFORM_PLUGIN_SVNREVERTJOB_H

#include "svnjobbase.h"

#include <QUrl>

class SvnInternalRevertJob;

class SvnRevertJob : public SvnJobBaseImpl<SvnInternalRevertJob>
{
    Q_OBJECT
public:
    explicit SvnRevertJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setLocations( const QList<QUrl>& locations );
    void setRecursive( bool );
};

#endif

