/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLANGTIDYPREFERENCES_H
#define CLANGTIDYPREFERENCES_H

#include <interfaces/configpage.h>

class ClangtidySettings;

class ClangtidyPreferences: public KDevelop::ConfigPage {
Q_OBJECT
public:
    explicit ClangtidyPreferences(KDevelop::IPlugin *plugin = nullptr, QWidget* parent = nullptr);
    ~ClangtidyPreferences() override;
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

};

#endif
