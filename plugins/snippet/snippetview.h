/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SNIPPETVIEW_H
#define SNIPPETVIEW_H

#include "ui_snippetview.h"

class SnippetFilterProxyModel;
class QStandardItem;
class SnippetPlugin;
class KAction;

/**
 * This class gets embedded into the right tool view by the SnippetPlugin.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Milian Wolff <mail@milianw.de>
 */
class SnippetView : public QWidget, public Ui::SnippetViewBase
{
    Q_OBJECT

public:
    explicit SnippetView(SnippetPlugin* plugin, QWidget* parent = 0);
    virtual ~SnippetView();

private slots:
    /**
     * Opens the "Add Repository" dialog.
     */
    void slotAddRepo();

    /**
     * Opens the "Edit repository" dialog.
     */
    void slotEditRepo();

    /**
     * Removes the selected repository from the disk.
     */
    void slotRemoveRepo();

    /**
     * Toggles activated status of the currently selected repository.
     */
    void slotToggleRepo();

    /**
     * Insert the selected snippet into the current file
     */
    void slotSnippetClicked(const QModelIndex & index);

    /**
     * Open the edit dialog for the selected snippet
     */
    void slotEditSnippet();

    /**
     * Removes the selected snippet from the tree and the filesystem
     */
    void slotRemoveSnippet();

    /**
     * Creates a new snippet and open the edit dialog for it
     */
    void slotAddSnippet();

    /**
     * Changes the filter of the proxy.
     */
    void slotFilterChanged();

    void contextMenu (const QPoint & pos);
    /// disables or enables available actions based on the currently selected item
    void validateActions();

private:
    QStandardItem* currentItem();

    SnippetPlugin* plugin_;
    SnippetFilterProxyModel* proxy_;

    KAction* m_addRepoAction;
    KAction* m_removeRepoAction;
    KAction* m_editRepoAction;
    KAction* m_toggleRepoAction;
    KAction* m_addSnippetAction;
    KAction* m_removeSnippetAction;
    KAction* m_editSnippetAction;
};

#endif

