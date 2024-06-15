/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakebuilderpreferences.h"

#include "ui_cmakebuilderpreferences.h"
#include "cmakebuilder.h"
#include "cmakebuilderconfig.h"
#include "cmakeutils.h"

#ifdef Q_OS_WIN
#include <kio_version.h>
#endif

CMakeBuilderPreferences::CMakeBuilderPreferences(KDevelop::IPlugin* plugin, QWidget* parent)
    : KDevelop::ConfigPage(plugin, CMakeBuilderSettings::self(), parent)
{
    m_prefsUi = new Ui::CMakeBuilderPreferences;
    m_prefsUi->setupUi(this);

#ifdef Q_OS_WIN
    m_prefsUi->kcfg_cmakeExecutable->setNameFilter(QStringLiteral("*.exe"));
#endif

    m_prefsUi->kcfg_cmakeExecutable->setToolTip(CMakeBuilderSettings::self()->cmakeExecutableItem()->whatsThis());
    m_prefsUi->label1->setToolTip(CMakeBuilderSettings::self()->cmakeExecutableItem()->whatsThis());

    const auto generators = CMake::supportedGenerators();
    for (const QString& generator : generators) {
        m_prefsUi->kcfg_generator->addItem(generator);
    }
}

CMakeBuilderPreferences::~CMakeBuilderPreferences()
{
    delete m_prefsUi;
}

QString CMakeBuilderPreferences::name() const
{
    return i18nc("@title:tab", "CMake");
}

QString CMakeBuilderPreferences::fullName() const
{
    return i18nc("@title:tab", "Configure Global CMake Settings");
}

QIcon CMakeBuilderPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("cmake"));
}

#include "moc_cmakebuilderpreferences.cpp"
