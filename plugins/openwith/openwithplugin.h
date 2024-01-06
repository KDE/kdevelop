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

class QMimeType;

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

private:
    void openApplication(const KService::Ptr& service);
    void openPart(const QString& pluginId, const QString& name);
    bool canOpenDefault() const;
    void openDefault();
    void rememberDefaultChoice(const QString& defaultId, const QString& name);

    /**
     * Update @a m_mimeType and @a m_defaultId based on @a m_urls.
     *
     * @return the updated MIME type
     */
    QMimeType updateMimeTypeForUrls();

    QList<QAction*> actionsForParts(QWidget* parent);
    QList<QAction*> actionsForApplications(QWidget* parent);
    QList<QUrl> m_urls;
    QString m_mimeType;
    QString m_defaultId;
};

#endif // KDEVPLATFORM_PLUGIN_OPENWITHPLUGIN_H
