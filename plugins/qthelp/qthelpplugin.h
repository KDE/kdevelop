/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPPLUGIN_H
#define QTHELPPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/idocumentationproviderprovider.h>

class QtHelpProvider;
class QtHelpQtDoc;
class QtHelpDocumentation;

class QtHelpPlugin : public KDevelop::IPlugin, public KDevelop::IDocumentationProviderProvider
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IDocumentationProviderProvider )
public:
    QtHelpPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~QtHelpPlugin() override;

    QList<KDevelop::IDocumentationProvider*> providers() override;
    QList<QtHelpProvider*> qtHelpProviderLoaded();

    /**
     * @return whether loading system Qt documentation is enabled in config
     */
    [[nodiscard]] bool loadsSystemQtDoc() const;

    bool isQtHelpAvailable() const;

    /**
     * @return @c false until all providers are fully set up
     *         and all configured documentation is loaded, @c true afterwards
     */
    bool isInitialized() const;

    int configPages() const override;
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

public Q_SLOTS:
    void readConfig();

Q_SIGNALS:
    void changedProvidersList() override;

private:
    void loadQtDocumentation(bool loadQtDoc);
    void searchHelpDirectory(QStringList& pathList, QStringList& nameList, QStringList& iconList,
                             const QString& searchDir);

    /**
     * Reset @a m_qtHelpProviders based on equal-size path, name and icon list arguments
     *
     * @param pathList a list of absolute paths to .qch files
     * @param nameList a list of provider names
     * @param iconList a list of provider icon names
     * @pre @p pathList.size() == @p nameList.size() && @p pathList.size() == @p iconList.size()
     */
    void loadQtHelpProvider(const QStringList& pathList, const QStringList& nameList, const QStringList& iconList);
    void removeUnusedHelpCollectionFiles() const;

    /// The list of loaded providers. This object is the parent of each provider,
    /// so they are not explicitly deleted in the destructor. But when a provider
    /// becomes obsolete and is removed from the list, it must be destroyed manually
    /// to prevent a leak until this object is destroyed (when the plugin is unloaded).
    ///
    /// Invariant: no two QtHelpProvider objects have equal namespace names. The namespace
    /// name determines the .qhc file name, so this prevents collection file conflicts.
    QList<QtHelpProvider*> m_qtHelpProviders;
    QtHelpQtDoc* m_qtDoc;
    bool m_loadSystemQtDoc;

    enum {
        ReadConfig = 1,
        LoadedQtDocumentation = 2
    };
    signed char m_initializationDone = 0;
};

#endif // QTHELPPLUGIN_H
