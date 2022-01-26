/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGINPREFERENCES_H
#define KDEVPLATFORM_PLUGINPREFERENCES_H

#include <QIcon>

#include <KLocalizedString>
#include <kcmutils_version.h>

#include <interfaces/configpage.h>

#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 91, 0)
class KPluginWidget;
#else
class KPluginSelector;
#endif

namespace KDevelop
{

class PluginPreferences : public ConfigPage
{
    Q_OBJECT
public:
    explicit PluginPreferences(QWidget*);

    QString name() const override { return i18n("Plugins"); };
    QString fullName() const override { return i18n("Configure Plugins"); };
    QIcon icon() const override { return QIcon::fromTheme(QStringLiteral("preferences-plugin")); }

public Q_SLOTS:
    void apply() override;
    void reset() override;
    void defaults() override;

private:
#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 91, 0)
    KPluginWidget* selector;
#else
    KPluginSelector* selector;
#endif
};

}
#endif
