/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATEENGINE_P_H
#define KDEVPLATFORM_TEMPLATEENGINE_P_H

#include <grantlee/engine.h>

namespace KDevelop {
class TemplateEnginePrivate
{
public:
    Grantlee::Engine engine;
};
}

#endif // KDEVPLATFORM_TEMPLATEENGINE_P_H
