/*************************************************************************************
 * This file is part of KDevPlatform                                                 *
 * Copyright 2016 Zhigalin Alexander <alexander@zhigalin.tk>                         *
 *                                                                                   *
 * This library is free software; you can redistribute it and/or                     *
 * modify it under the terms of the GNU Lesser General Public                        *
 * License as published by the Free Software Foundation; either                      *
 * version 2.1 of the License, or (at your option) version 3, or any                 *
 * later version accepted by the membership of KDE e.V. (or its                      *
 * successor approved by the membership of KDE e.V.), which shall                    *
 * act as a proxy defined in Section 6 of version 3 of the license.                  *
 *                                                                                   *
 * This library is distributed in the hope that it will be useful,                   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                 *
 * Lesser General Public License for more details.                                   *
 *                                                                                   *
 * You should have received a copy of the GNU Lesser General Public                  *
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.     *
 *************************************************************************************/

#include "colorschemechooser.h"

#include <QAction>
#include <QMenu>
#include <QModelIndex>

#include <KActionMenu>
#include <KColorSchemeManager>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

#include <interfaces/isession.h>
#include "mainwindow.h"
#include "core.h"
#include "debug.h"

namespace KDevelop
{

ColorSchemeChooser::ColorSchemeChooser(QObject* parent)
    : QAction(parent)
{
    auto manager = new KColorSchemeManager(parent);

    const auto scheme(currentSchemeName());
    qCDebug(SHELL) << "Color scheme : " << scheme;

    auto selectionMenu = manager->createSchemeSelectionMenu(scheme, this);

    connect(selectionMenu->menu(), &QMenu::triggered,
            this, &ColorSchemeChooser::slotSchemeChanged);

    manager->activateScheme(manager->indexForScheme(scheme));

    setMenu(selectionMenu->menu());
    menu()->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));
    menu()->setTitle(i18n("&Color Scheme"));
}

QString ColorSchemeChooser::loadCurrentScheme() const
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, "UiSettings");
#if KCONFIGWIDGETS_VERSION >= QT_VERSION_CHECK(5, 67, 0)
    // Since 5.67 KColorSchemeManager includes a system color scheme option that reacts to system
    // scheme changes. This scheme will be activated if we pass an empty string to KColorSchemeManager
    // So no need anymore to read the the current global scheme ourselves if no custom one is configured.
    return cg.readEntry("ColorScheme");
#else
    return cg.readEntry("ColorScheme", currentDesktopDefaultScheme());
#endif
}

void ColorSchemeChooser::saveCurrentScheme(const QString &name)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, "UiSettings");
    cg.writeEntry("ColorScheme", name);
    cg.sync();
}

#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5, 67, 0)
QString ColorSchemeChooser::currentDesktopDefaultScheme() const
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    KConfigGroup group(config, "General");
    const QString scheme = group.readEntry("ColorScheme", QStringLiteral("Breeze"));
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
    QStringLiteral("color-schemes/%1.colors").arg(scheme));
    KSharedConfigPtr schemeFile = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    const QString name = KConfigGroup(schemeFile, "General").readEntry("Name", scheme);
    return name;
}
#endif

QString ColorSchemeChooser::currentSchemeName() const
{
    if(!menu()) return loadCurrentScheme();

    QAction* const action = menu()->activeAction();

    if(action) return KLocalizedString::removeAcceleratorMarker(action->text());
#if KCONFIGWIDGETS_VERSION >= QT_VERSION_CHECK(5, 67, 0)
    // See above
    return QString();
#else
    return currentDesktopDefaultScheme();
#endif
}

void ColorSchemeChooser::slotSchemeChanged(QAction* triggeredAction)
{
    saveCurrentScheme(KLocalizedString::removeAcceleratorMarker(triggeredAction->text()));
}

}  // namespace KDevelop
