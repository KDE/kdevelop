/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEWPLUGIN_H
#define KDEVPLATFORM_PLUGIN_KDEVDOCUMENTVIEWPLUGIN_H

#include <interfaces/iplugin.h>

class KDevDocumentViewPluginFactory;

class KDevDocumentViewPlugin: public KDevelop::IPlugin
{
    Q_OBJECT
public:
    enum RefreshPolicy
    {
        Refresh,
        NoRefresh,
        ForceRefresh
    };

public:
    KDevDocumentViewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~KDevDocumentViewPlugin() override;
    void unload() override;

private:
    KDevDocumentViewPluginFactory* factory;
};

#endif

