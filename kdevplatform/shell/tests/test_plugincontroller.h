/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_PLUGINCONTROLLER_H
#define KDEVPLATFORM_TEST_PLUGINCONTROLLER_H

#include <QObject>

namespace KDevelop
{
class Core;
class PluginController;
}

class TestPluginController : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void loadUnloadPlugin();
    void loadFromExtension();
    void pluginInfo();
    void benchPluginForExtension();

private:
    KDevelop::PluginController* m_pluginCtrl;
};

#endif // KDEVPLATFORM_TEST_PLUGINCONTROLLER_H
