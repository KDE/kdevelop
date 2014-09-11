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

#ifndef MAKEBUILDERPREFERENCES_H
#define MAKEBUILDERPREFERENCES_H

#include <QUrl>

#include <kcmodule.h>

class QWidget;
class QStringList;
class CMakeBuilderSettings;
namespace Ui { class CMakeBuilderConfig; }

class CMakeBuilderPreferences : public KCModule
{
    Q_OBJECT

public:
    explicit CMakeBuilderPreferences(QWidget* parent = 0, const QVariantList& args = QVariantList());
    ~CMakeBuilderPreferences();
    
    virtual void save();
    virtual void load();
    virtual void defaults();

public slots:
    void generatorChanged(const QString& generator);

private:
    Ui::CMakeBuilderConfig* m_prefsUi;
};

#endif
