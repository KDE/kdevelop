/* KDevelop Project Settings
 *
 * Copyright 2008 Andreas Pakulat <apaku@gmx.de>
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

#ifndef KDEVPLATFORM_PLUGINPREFERENCES_H
#define KDEVPLATFORM_PLUGINPREFERENCES_H

#include <QIcon>
#include <KLocalizedString>

#include <interfaces/configpage.h>

class KPluginSelector;

namespace KDevelop
{

class PluginPreferences : public ConfigPage
{
    Q_OBJECT
public:
    explicit PluginPreferences(QWidget*);

    QString name() const override { return i18n("Plugins"); };
    QString fullName() const override { return i18n("Configure Plugins"); };
    QIcon icon() const override { return QIcon::fromTheme(QStringLiteral("preferences-plugin")); }

public Q_SLOTS:
    void apply() override;
    void reset() override;
    void defaults() override;

private:
    KPluginSelector* selector;
};

}
#endif
