/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNMOVEJOB_H
#define KDEVPLATFORM_PLUGIN_SVNMOVEJOB_H

#include "svnjobbase.h"

class QUrl;

class SvnInternalMoveJob;

class SvnMoveJob : public SvnJobBaseImpl<SvnInternalMoveJob>
{
    Q_OBJECT
public:
    explicit SvnMoveJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setSourceLocation( const QUrl &location );
    void setDestinationLocation( const QUrl &location );
    void setForce( bool );
};


#endif
