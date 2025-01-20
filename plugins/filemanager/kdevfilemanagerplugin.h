/*
    SPDX-FileCopyrightText: 2006 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVFILEMANAGERPLUGIN_H
#define KDEVPLATFORM_PLUGIN_KDEVFILEMANAGERPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariantList>

class FileManager;

class KDevFileManagerPlugin: public KDevelop::IPlugin {
    Q_OBJECT
public:
    explicit KDevFileManagerPlugin(QObject* parent, const KPluginMetaData& metaData,
                                   const QVariantList& args = QVariantList());
    ~KDevFileManagerPlugin() override;

    void unload() override;

private:
    class KDevFileManagerViewFactory *m_factory;

};

#endif

