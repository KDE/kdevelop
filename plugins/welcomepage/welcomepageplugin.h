/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVWELCOMEPAGEPLUGIN_H
#define KDEVPLATFORM_PLUGIN_KDEVWELCOMEPAGEPLUGIN_H

// KDevPlatform
#include <interfaces/iplugin.h>
// Qt
#include <QPointer>

class KDevWelcomePagePlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    KDevWelcomePagePlugin(QObject* parent, const KPluginMetaData& metaData, const QList<QVariant>&);

public:
    void unload() override;

private:
    QPointer<QWidget> m_welcomePageWidget;
};

#endif
