/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>
    Copyright 2021 BogDan Vatra <bogdan@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "gnadvancedsettings.h"

#include "ui_gnadvancedsettings.h"

using namespace KDevelop;

GNAdvancedSettings::GNAdvancedSettings(QWidget* parent)
    : QWidget(parent)
{
    m_ui = new Ui::GNAdvancedSettings;
    m_ui->setupUi(this);
    m_ui->container->hide();
}

GNAdvancedSettings::~GNAdvancedSettings()
{
    delete m_ui;
}

GNAdvancedSettings::Data GNAdvancedSettings::getConfig() const
{
    Data res;

    res.args = m_ui->i_gnArgs->text();
    res.gn = Path(m_ui->i_gnExe->url());

    return res;
}

void GNAdvancedSettings::setConfig(const GNAdvancedSettings::Data& conf)
{
    m_ui->i_gnArgs->setText(conf.args);
    m_ui->i_gnExe->setUrl(conf.gn.toUrl());
}

void GNAdvancedSettings::updated()
{
    emit configChanged();
}

/// Check if gn has changed since the last call
bool GNAdvancedSettings::hasGNChanged()
{
    if (m_gnOldPath != Path(m_ui->i_gnExe->url())) {
        m_gnOldPath = Path(m_ui->i_gnExe->url()); // Reset
        return true;
    }

    return false;
}
