/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <interfaces/iplugin.h>

class KJob;

namespace Heaptrack
{

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Plugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~Plugin() override;

    int configPages() const override;
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

private:
    void launchHeaptrack();
    void attachHeaptrack();

    void jobFinished(KJob* job);

    QAction* m_launchAction;
    QAction* m_attachAction;
};

}
