/***************************************************************************
 *   Copyright 2020  Morten Danielsen Volden                               *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef SHELLCHECK_GLOBAL_CONFIG_PAGE_H
#define SHELLCHECK_GLOBAL_CONFIG_PAGE_H

#include <interfaces/configpage.h>

namespace shellcheck
{

class GlobalConfigPage: public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    GlobalConfigPage(KDevelop::IPlugin* plugin, QWidget* parent);
    ~GlobalConfigPage() override;

    KDevelop::ConfigPage::ConfigPageType configPageType() const override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;
};

}

#endif
