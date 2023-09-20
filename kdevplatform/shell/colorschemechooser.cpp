/*
    SPDX-FileCopyrightText: 2016 Zhigalin Alexander <alexander@zhigalin.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "colorschemechooser.h"

#include <QAction>
#include <QMenu>
#include <QModelIndex>

#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
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

    auto selectionMenu = KColorSchemeMenu::createMenu(manager, this);

    connect(selectionMenu->menu(), &QMenu::triggered,
            this, &ColorSchemeChooser::slotSchemeChanged);

    manager->activateScheme(manager->indexForScheme(scheme));

    setMenu(selectionMenu->menu());
    menu()->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));
    menu()->setTitle(i18n("&Window Color Scheme"));
}

QString ColorSchemeChooser::loadCurrentScheme() const
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, "UiSettings");
    // Since 5.67 KColorSchemeManager includes a system color scheme option that reacts to system
    // scheme changes. This scheme will be activated if we pass an empty string to KColorSchemeManager
    // So no need anymore to read the current global scheme ourselves if no custom one is configured.
    return cg.readEntry("ColorScheme");
}

void ColorSchemeChooser::saveCurrentScheme(const QString &name)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, "UiSettings");
    cg.writeEntry("ColorScheme", name);
    cg.sync();
}

QString ColorSchemeChooser::currentSchemeName() const
{
    if(!menu()) return loadCurrentScheme();

    QAction* const action = menu()->activeAction();

    if(action) return KLocalizedString::removeAcceleratorMarker(action->text());
    // See above
    return QString();
}

void ColorSchemeChooser::slotSchemeChanged(QAction* triggeredAction)
{
    saveCurrentScheme(KLocalizedString::removeAcceleratorMarker(triggeredAction->text()));
}

}  // namespace KDevelop

#include "moc_colorschemechooser.cpp"
