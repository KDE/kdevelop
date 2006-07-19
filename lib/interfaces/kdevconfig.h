/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVCONFIG_H
#define KDEVCONFIG_H

#include <QObject>
#include <kconfig.h>

#include "kdevexport.h"

class KDEVINTERFACES_EXPORT KDevConfig: public QObject
{
    Q_OBJECT
public:
    KDevConfig( QObject *parent = 0 );
    virtual ~KDevConfig();

    static KSharedConfig::Ptr sharedGlobalProject();

    static KSharedConfig::Ptr sharedLocalProject();

    static KSharedConfig::Ptr sharedStandard();

    static KConfig *globalProject();

    static KConfig *localProject();

    static KConfig *standard();

private:
    static KDevConfig *getInstance();
    static KDevConfig *s_instance;
//     KSharedConfig::Ptr m_global;
//     KSharedConfig::Ptr m_local;
//     KSharedConfig::Ptr m_standard;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
