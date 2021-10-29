/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_PLUGINENABLING_H
#define KDEVPLATFORM_TEST_PLUGINENABLING_H

#include <QObject>

namespace KDevelop
{
class PluginController;
}

class TestPluginEnabling : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void loadPluginCustomDefaults_data();
    void loadPluginCustomDefaults();
    void loadPluginNormalDefaults_data();
    void loadPluginNormalDefaults();

private:
    void loadPlugin(const QString& pluginId, bool shouldBeEnabled);

private:
    KDevelop::PluginController* m_pluginCtrl;
};

#endif // KDEVPLATFORM_TEST_PLUGINENABLING_H
