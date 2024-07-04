/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTPLUGIN_H
#define KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariantList>
#include <KConfigGroup>
#include <QUrl>

class ExternalScriptItem;

class QStandardItem;
class QStandardItemModel;
class QModelIndex;

class ExternalScriptPlugin
    : public KDevelop::IPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kdevelop.ExternalScriptPlugin")

public:
    explicit ExternalScriptPlugin(QObject* parent, const KPluginMetaData& metaData,
                                  const QVariantList& args = QVariantList());

    ~ExternalScriptPlugin() override;
    void unload() override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    static ExternalScriptPlugin* self();

    /**
     * @return The model storing all external scripts managed by this plugin.
     * @NOTE: always append() items, never insert in the middle!
     */
    QStandardItemModel* model() const;

    /**
     * Executes @p script.
     */
    void execute(ExternalScriptItem* item, const QUrl& url) const;

    /**
     * Executes @p script.
     */
    void execute(ExternalScriptItem* item) const;

    /**
     * Returns config group to store all settings for this plugin in.
     */
    KConfigGroup getConfig() const;

    /**
     * Saves the @p script to the config and updates the key
     */
    void saveItem(const ExternalScriptItem* item);

public Q_SLOTS:
    void executeScriptFromActionData() const;

    /**
     * Executes the command (Used by the shell-integration)
     * */
    Q_SCRIPTABLE bool executeCommand(const QString& command, const QString& workingDirectory) const;

    /**
     * Executes the command synchronously and returns the output text (Used by the shell-integration)
     * */
    Q_SCRIPTABLE QString executeCommandSync(const QString& command, const QString& workingDirectory) const;

private Q_SLOTS:
    void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void rowsInserted(const QModelIndex& parent, int start, int end);
    void executeScriptFromContextMenu() const;

private:
    /// @param row row in the model for the item to save
    void saveItemForRow(int row);

    /**
     * Sets up unique keys for items in the range [start, end]
     * @param start start of the range
     * @param end end of the range
     */
    void setupKeys(int start, int end);

    QStandardItemModel* m_model;
    QList<QUrl> m_urls;
    static ExternalScriptPlugin* m_self;

    class ExternalScriptViewFactory* m_factory;
};

#endif // KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTPLUGIN_H
