/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SESSIONCONFIGSKELETON_H
#define KDEVPLATFORM_SESSIONCONFIGSKELETON_H

#include "../core.h"
#include "../session.h"

#include <KConfigSkeleton>

namespace KDevelop
{

class SessionConfigSkeleton : public KConfigSkeleton
{
public:
    explicit SessionConfigSkeleton( const QString& )
        : KConfigSkeleton( Core::self()->activeSession()->config() )
    {
    }
};

}

#endif
