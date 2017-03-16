/* KDevelop CCMake Support
 *
 * Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef CMAKEBUILDERPREFERENCES_H
#define CMAKEBUILDERPREFERENCES_H

#include <interfaces/configpage.h>

class QWidget;
class CMakeBuilderSettings;
namespace Ui { class CMakeBuilderPreferences; }

class CMakeBuilderPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    explicit CMakeBuilderPreferences(KDevelop::IPlugin* plugin, QWidget* parent = nullptr);
    ~CMakeBuilderPreferences() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

private:
    Ui::CMakeBuilderPreferences* m_prefsUi;
};

#endif
