/*
 * This file is part of KDevelop
 * Copyright 2012 André Stein <andre.stein@rwth-aachen.de>
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KDEVPLATFORM_PLUGIN_SWITCHTOBUDDY_H
#define KDEVPLATFORM_PLUGIN_SWITCHTOBUDDY_H

#include <interfaces/iplugin.h>
#include <interfaces/contextmenuextension.h>

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
 * If a language plugin either doens't provide the @c IBuddyDocumentFinder
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
    SwitchToBuddyPlugin( QObject *parent, const QVariantList & = QVariantList());
    ~SwitchToBuddyPlugin();

    virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    virtual void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions) override;

private slots:
    /**
     * Context menu slot which switches to the QUrl provided
     * in the data part of the sending QAction.
     */
    void switchToBuddy(const QString& url);

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

private:
    class QSignalMapper* m_signalMapper;
};


#endif // KDEVPLATFORM_PLUGIN_SWITCHTOBUDDY_H
