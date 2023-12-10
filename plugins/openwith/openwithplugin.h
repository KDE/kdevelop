/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_OPENWITHPLUGIN_H
#define KDEVPLATFORM_PLUGIN_OPENWITHPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariantList>

#include <KService>

#include "iopenwith.h"

class OpenWithPlugin : public KDevelop::IPlugin, public KDevelop::IOpenWith
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IOpenWith )
public:
    OpenWithPlugin( QObject* parent, const QVariantList& args );
    ~OpenWithPlugin() override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

protected:
    void openFilesInternal( const QList<QUrl>& files ) override;

private Q_SLOTS:
    void open( const QString& storageId );
    void openService( const KService::Ptr& service );
    void openDefault();

private:
    QList<QAction*> actionsForServiceType(const QString& serviceType, QWidget* parent);
    QList<QUrl> m_urls;
    QString m_mimeType;
};

#endif // KDEVPLATFORM_PLUGIN_OPENWITHPLUGIN_H
