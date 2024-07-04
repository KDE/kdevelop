/*
    SPDX-FileCopyrightText: 2003, 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVKONSOLEVIEWPLUGIN_H
#define KDEVPLATFORM_PLUGIN_KDEVKONSOLEVIEWPLUGIN_H

#include <interfaces/iplugin.h>

#include <QVariant>

class KDevKonsoleViewFactory;
class KPluginFactory;

class KDevKonsoleViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    KDevKonsoleViewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~KDevKonsoleViewPlugin() override;

    void unload() override;

    KPluginFactory* konsoleFactory() const;

private:
    KPluginFactory* m_konsoleFactory = nullptr;
    KDevKonsoleViewFactory* m_viewFactory = nullptr;
};

#endif

