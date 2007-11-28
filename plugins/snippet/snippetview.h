/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SNIPPETVIEW_H__
#define __SNIPPETVIEW_H__

#include "ui_snippetview.h"

class SnippetFilterProxyModel;
class QStandardItem;
class SnippetPlugin;

/**
 * This class gets embedded into the right tool view by the SnippetPlugin.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class SnippetView : public QWidget, public Ui::SnippetViewBase
{
    Q_OBJECT

public:
    explicit SnippetView(SnippetPlugin* plugin, QWidget* parent = 0);
    virtual ~SnippetView();

public slots:
    /**
     * Opens the "Add Repository" dialog and inserts the selected directory into the model
     */
    void slotAddRepo();

    /**
     * Insert the selected snippet into the current file
     */
    void slotSnippetClicked (const QModelIndex & index);

    /**
     * Open the edit dialog for the selected snippet
     */
    void slotEditSnippet();

    /**
     * Removes the selected snippet from the tree and the filesystem
     */
    void slotDeleteSnippet();

    /**
     * Creates a new snippet and open the edit dialog for it
     */
    void slotAddSnippet();

    /**
     * Changes the filter of the proxy.
     */
    void slotFilterChanged();

    /**
     * Removes the selected repository from the model.
     * Calling this method does not have any impact on the directories and files on the disk.
     */
    void slotHideRepository();

    /**
     * Changes where the repository is located on the disk.
     */
    void slotMoveRepository();

    /**
     * Removes the selected repository from the disk.
     */
    void slotRemoveRepo();

    /**
     * Creates a new subrepository
     */
    void slotCreateSubRepo();

    /**
     * Clears the filter input field
     */
    void slotClearFilter();

signals:
    /**
     * This signal gets emitted whenever the user requests a contextmenu.
     * The slot that gets connected to this signal can insert additional entries
     * to the contextmenu @p menu. The parameter @p item is either a Snippet,
     * a SnippetRepository or NULL, depending on where the users requested the contextmenu.
     */
    void popupContextMenu( QMenu* menu, QStandardItem* item );

private slots:
    void contextMenu (const QPoint & pos);

private:
    QStandardItem* currentItem();

    SnippetPlugin* plugin_;
    SnippetFilterProxyModel* proxy_;
};

#endif

