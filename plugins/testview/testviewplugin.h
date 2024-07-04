/*
    SPDX-FileCopyrightText: 2012 Miha ?an?ula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_TESTVIEWPLUGIN_H
#define KDEVPLATFORM_PLUGIN_TESTVIEWPLUGIN_H

#include "interfaces/iplugin.h"
#include <QVariantList>

class TestToolViewFactory;

class TestViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit TestViewPlugin(QObject* parent, const KPluginMetaData& metaData,
                            const QVariantList& args = QVariantList());
    ~TestViewPlugin() override;

    void unload() override;

private:
    TestToolViewFactory* m_viewFactory;

private Q_SLOTS:
    void runAllTests();
    void stopRunningTests();
    void jobStateChanged();
};

#endif // KDEVPLATFORM_PLUGIN_TESTVIEWPLUGIN_H
