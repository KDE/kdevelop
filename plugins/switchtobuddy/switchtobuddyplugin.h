/*
    SPDX-FileCopyrightText: 2012 André Stein <andre.stein@rwth-aachen.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SWITCHTOBUDDYPLUGIN_H
#define KDEVPLATFORM_PLUGIN_SWITCHTOBUDDYPLUGIN_H

#include <interfaces/iplugin.h>

#include <QVariantList>

/**
 * @short Implements a context menu extension in an editor context which provides
 *  an action that allows switching to associated buddy documents.
 *
 * Using the @c IBuddyDocumentFinder interface, the current document's
 * language plugin provides potential buddy candidates. Depending on their
 * existence on the file system the @c SwitchToBuddyPlugin
 * enables a 'Switch To XXX' action which opens that buddy document
 * using the @c IDocumentController.
 *
 * If a language plugin either doesn't provide the @c IBuddyDocumentFinder
 * interface or no buddy exists on the file system, no context menu
 * extension is performed.
 *
 * @see IBuddyDocumentFinder
 * @see IDocumentController
 */
class SwitchToBuddyPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit SwitchToBuddyPlugin(QObject* parent, const KPluginMetaData& metaData,
                                 const QVariantList& = QVariantList());
    ~SwitchToBuddyPlugin() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;
    void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions) override;

private Q_SLOTS:
    /**
     * Context menu slot which switches to the QUrl provided
     * in the data part of the sending QAction.
     */
    void switchToBuddy(const QString& url);

    /**
     * Switch between header and source files
     */
    void switchHeaderSource();

    /**
     * @brief Switch between definitions and declarations
     *
     * E.g. if the cursor in the currently active view points to an implementation file
     * this shortcut will open the header document (or any buddy file).
     *
     * Furthermore, if the cursor points to a definition, and the buddy document contains its declaration,
     * the cursor will be also set to the declaration's position when the buddy document is opened
     */
    void switchDefinitionDeclaration();
};


#endif // KDEVPLATFORM_PLUGIN_SWITCHTOBUDDYPLUGIN_H
