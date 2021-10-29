/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SESSIONCONFIGSKELETON_H
#define SESSIONCONFIGSKELETON_H

#include <interfaces/icore.h>
#include <interfaces/isession.h>

#include <KConfigSkeleton>

using namespace KDevelop;

class SessionConfigSkeleton : public KConfigSkeleton
{
public:
    explicit SessionConfigSkeleton( const QString& )
        : KConfigSkeleton( ICore::self()->activeSession()->config() )
    {
    }
};

#endif
