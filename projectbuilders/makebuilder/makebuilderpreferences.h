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

#ifndef MAKEBUILDERPREFERENCES_H
#define MAKEBUILDERPREFERENCES_H

#include <project/projectkcmodule.h>
#include <kurl.h>

class QWidget;
class QStringList;
class MakeBuilderSettings;
namespace Ui { class MakeConfig; }

class MakeBuilderPreferences : public ProjectKCModule<MakeBuilderSettings>
{
    Q_OBJECT

public:
    explicit MakeBuilderPreferences(QWidget* parent = 0, const QVariantList& args = QVariantList());
    ~MakeBuilderPreferences();
    void load();
    void save();
    void defaults();

    static QString standardMakeComannd();

private:
    Ui::MakeConfig* m_prefsUi;
};

#endif
