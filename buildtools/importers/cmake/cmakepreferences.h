/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEPREFERENCES_H
#define CMAKEPREFERENCES_H

#include <kcmodule.h>

class QWidget;
class QStringList;

namespace Ui { class CMakeBuildSettings; }

/**
 * @author Matt Rogers <mattr@kde.org>
 */
class CMakePreferences : public KCModule
{
public:
    CMakePreferences(QWidget* parent = 0, const QStringList& args = QStringList());
    ~CMakePreferences();

    virtual void load();
    virtual void save();
    virtual void defaults();

private slots:
    void settingsChanged();

private:
    Ui::CMakeBuildSettings* m_prefsUi;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on;
