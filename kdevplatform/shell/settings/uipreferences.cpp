/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "uipreferences.h"

#include <KLocalizedString>

#include "../core.h"
#include "../uicontroller.h"
#include "ui_uiconfig.h"
#include "uiconfig.h"

#include <sublime/mainwindow.h>

using namespace KDevelop;

UiPreferences::UiPreferences(QWidget* parent)
    : ConfigPage(nullptr, UiConfig::self(), parent)
{
    m_uiconfigUi = new Ui::UiConfig();
    m_uiconfigUi->setupUi(this);
}

UiPreferences::~UiPreferences()
{
    delete m_uiconfigUi;
}

void UiPreferences::apply()
{
    KDevelop::ConfigPage::apply();

    UiController *uiController = Core::self()->uiControllerInternal();
    const auto windows = uiController->mainWindows();
    for (Sublime::MainWindow* window : windows) {
        window->loadUiPreferences();
    }
    uiController->loadSettings();
}

QString UiPreferences::name() const
{
    return i18n("User Interface");
}

QIcon UiPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("preferences-desktop-theme"));
}

QString UiPreferences::fullName() const
{
    return i18n("Configure User Interface");
}

#include "moc_uipreferences.cpp"
