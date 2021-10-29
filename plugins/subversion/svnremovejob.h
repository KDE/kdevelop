/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNREMOVEJOB_H
#define KDEVPLATFORM_PLUGIN_SVNREMOVEJOB_H


#include "svnjobbase.h"

#include <QUrl>

class SvnInternalRemoveJob;

class SvnRemoveJob : public SvnJobBaseImpl<SvnInternalRemoveJob>
{
    Q_OBJECT
public:
    explicit SvnRemoveJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setLocations( const QList<QUrl>& locations );
    void setForce( bool force );
};

#endif

