/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "configentry.h"

#include <QList>
#include <KConfigGroup>

#include <kdemacros.h>

class KConfig;
namespace KDevelop
{
class IProject;
}

/// @brief: Class responsible for reading/writing custom defines and includes from/to the disk.
class KDE_EXPORT SettingsManager
{
private:
    SettingsManager();
public:
    static SettingsManager* self();

    QList<ConfigEntry> readSettings(KConfig* cfg) const;

    void writeSettings(KConfig* cfg, const QList<ConfigEntry>& paths) const;
};

#endif // SETTINGSMANAGER_H
