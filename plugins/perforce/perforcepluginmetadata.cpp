/***************************************************************************
 *   Copyright 2015 Morten Danielsen Volden <mvolden2@gmail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "perforceplugin.h"

#include <KPluginFactory>


// This file only exists so that the tests can be built:
// test_kdevperforce builds perforceplugin.cpp again but in a different directory.
// This means that the kdevperforce.json file is no longer found.
// Since the JSON metadata is not needed in the test, we simply move
// the K_PLUGIN_FACTORY_WITH_JSON to a separate file.

K_PLUGIN_FACTORY_WITH_JSON(KdevPerforceFactory, "kdevperforce.json", registerPlugin<PerforcePlugin>();)

#include "perforcepluginmetadata.moc"
