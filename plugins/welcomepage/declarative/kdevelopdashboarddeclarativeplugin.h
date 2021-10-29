/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVPLATFORMDECLARATIVEPLUGIN_H
#define KDEVPLATFORM_PLUGIN_KDEVPLATFORMDECLARATIVEPLUGIN_H

#include <QQmlExtensionPlugin>

class KDevplatformDeclarativePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char* uri) override;
};

#endif // KDEVPLATFORM_PLUGIN_KDEVPLATFORMDECLARATIVEPLUGIN_H
