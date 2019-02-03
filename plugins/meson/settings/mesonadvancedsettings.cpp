/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

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

#include "mesonadvancedsettings.h"

#include "ui_mesonadvancedsettings.h"

using namespace KDevelop;

MesonAdvancedSettings::MesonAdvancedSettings(QWidget* parent)
    : QWidget(parent)
{
    m_ui = new Ui::MesonAdvancedSettings;
    m_ui->setupUi(this);
    m_ui->container->hide();
}

MesonAdvancedSettings::~MesonAdvancedSettings()
{
    delete m_ui;
}

MesonAdvancedSettings::Data MesonAdvancedSettings::getConfig() const
{
    Data res;

    res.args = m_ui->i_mesonArgs->text();
    res.backend = m_ui->i_backend->currentText();
    res.meson = Path(m_ui->i_mesonExe->url());

    return res;
}

void MesonAdvancedSettings::setConfig(const MesonAdvancedSettings::Data& conf)
{
    m_ui->i_mesonArgs->setText(conf.args);
    m_ui->i_mesonExe->setUrl(conf.meson.toUrl());
    m_ui->i_backend->setCurrentIndex(std::max(0, m_backendList.indexOf(conf.backend)));
}

void MesonAdvancedSettings::setSupportedBackends(const QStringList& backends)
{
    m_backendList = backends;
    m_ui->i_backend->clear();
    m_ui->i_backend->addItems(m_backendList);
}

void MesonAdvancedSettings::updated()
{
    emit configChanged();
}

/// Check if meson has changed since the last call
bool MesonAdvancedSettings::hasMesonChanged()
{
    if(m_mesonOldPath != Path(m_ui->i_mesonExe->url())) {
        m_mesonOldPath = Path(m_ui->i_mesonExe->url()); // Reset
        return true;
    }

    return false;
}
