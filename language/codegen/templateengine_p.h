/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_TEMPLATEENGINE_P_H
#define KDEVPLATFORM_TEMPLATEENGINE_P_H

#include <grantlee/engine.h>

#include <grantlee/grantlee_version.h>
#include <kdeversion.h>
#define WITH_SMART_TRIM KDE_MAKE_VERSION(GRANTLEE_VERSION_MAJOR, GRANTLEE_VERSION_MINOR, GRANTLEE_VERSION_PATCH) >= 0x000200

namespace KDevelop {

struct TemplateEnginePrivate
{
    Grantlee::Engine engine;
};

}

#endif // KDEVPLATFORM_TEMPLATEENGINE_P_H
