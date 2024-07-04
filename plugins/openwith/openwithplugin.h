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

namespace OpenWithUtils {
class FileOpener
{
public:
    FileOpener() = default;
    FileOpener(const KService::Ptr& service);
    static FileOpener fromPartId(const QString& partId);

    static FileOpener fromConfigEntryValue(const QString& value);
    QString toConfigEntryValue() const;

    /**
     * @return whether this opener object is valid
     * @note Most member functions and operators require valid opener(s) as a precondition.
     */
    bool isValid() const;

    bool isPart() const;
    const QString& id() const;

    /**
     * @return non-null service pointer for a valid application opener
     * @pre !isPart()
     */
    const KService::Ptr& service() const;

private:
    explicit FileOpener(bool isPart, const QString& id);

    bool m_isPart = false;
    QString m_id;
    KService::Ptr m_service; ///< a cached service pointer
};

bool operator==(const FileOpener&, const FileOpener&);
} // namespace OpenWithUtils

class OpenWithPlugin : public KDevelop::IPlugin, public KDevelop::IOpenWith
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IOpenWith )
public:
    OpenWithPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~OpenWithPlugin() override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

protected:
    void openFilesInternal( const QList<QUrl>& files ) override;

private:
    void openApplication(const KService::Ptr& service);
    void openPart(const QString& pluginId, const QString& name);
    bool canOpenDefault() const;
    void openDefault() const;
    void delegateToParts(const QString& pluginId) const;
    void delegateToExternalApplication(const KService::Ptr& service) const;
    void rememberDefaultChoice(const OpenWithUtils::FileOpener& opener, const QString& name);

    /**
     * Update @a m_mimeType and @a m_defaultOpener based on @a m_urls.
     *
     * @return the updated MIME type
     */
    QMimeType updateMimeTypeForUrls();

    QList<QAction*> actionsForParts(QWidget* parent);
    QList<QAction*> actionsForApplications(QWidget* parent);
    QList<QUrl> m_urls;
    QString m_mimeType;
    OpenWithUtils::FileOpener m_defaultOpener;
};

#endif // KDEVPLATFORM_PLUGIN_OPENWITHPLUGIN_H
