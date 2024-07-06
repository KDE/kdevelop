/*
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    m_ui->i_backend->setCurrentIndex(std::max(0, static_cast<int>(m_backendList.indexOf(conf.backend))));
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
    if (m_mesonOldPath != Path(m_ui->i_mesonExe->url())) {
        m_mesonOldPath = Path(m_ui->i_mesonExe->url()); // Reset
        return true;
    }

    return false;
}

#include "moc_mesonadvancedsettings.cpp"
