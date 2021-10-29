/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGINPREFERENCES_H
#define KDEVPLATFORM_PLUGINPREFERENCES_H

#include <QIcon>
#include <KLocalizedString>

#include <interfaces/configpage.h>

class KPluginSelector;

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
    KPluginSelector* selector;
};

}
#endif
