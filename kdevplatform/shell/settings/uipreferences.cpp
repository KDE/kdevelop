/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "uipreferences.h"

#include <KLocalizedString>

#include "../core.h"
#include "../mainwindow.h"
#include "../uicontroller.h"
#include "ui_uiconfig.h"
#include "uiconfig.h"

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
        (static_cast<KDevelop::MainWindow*>(window))->loadSettings();
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

