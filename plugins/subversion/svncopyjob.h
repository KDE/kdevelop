/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNCOPYJOB_H
#define KDEVPLATFORM_PLUGIN_SVNCOPYJOB_H

#include "svnjobbase.h"

#include <QUrl>

class SvnInternalCopyJob;

class SvnCopyJob : public SvnJobBaseImpl<SvnInternalCopyJob>
{
    Q_OBJECT
public:
    explicit SvnCopyJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setSourceLocation( const QUrl &location );
    void setDestinationLocation( const QUrl &location );
};


#endif
