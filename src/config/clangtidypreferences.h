/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef CLANGTIDYPREFERENCES_H
#define CLANGTIDYPREFERENCES_H

#include <interfaces/configpage.h>

namespace Ui
{
class ClangtidySettings;
};
/**
 * \class
 * \brief Implements the session configuration page for clang-tidy.
 */
class ClangtidyPreferences : public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    explicit ClangtidyPreferences(KDevelop::IPlugin* plugin = nullptr, QWidget* parent = nullptr);
    ~ClangtidyPreferences() override;
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public slots:
    void apply() override;

private:
    Ui::ClangtidySettings* ui;
};

#endif
