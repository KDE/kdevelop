/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef ANDROIDPLUGIN_H
#define ANDROIDPLUGIN_H

#include <interfaces/iplugin.h>

class AndroidPreferencesSettings;

class AndroidPlugin : public KDevelop::IPlugin
{
Q_OBJECT
public:
    AndroidPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~AndroidPlugin() override;

    int configPages() const override;
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

private:
    QScopedPointer<AndroidPreferencesSettings> m_settings;
};

#endif
