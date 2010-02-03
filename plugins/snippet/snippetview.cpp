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

#include "snippetview.h"


#include <QContextMenuEvent>
#include <KMenu>
#include <KMessageBox>
#include <KAction>

#include "snippet.h"
#include "snippetplugin.h"
#include "snippetrepository.h"
#include "snippetstore.h"
#include "editrepository.h"
#include "editsnippet.h"
#include "snippetfilterproxymodel.h"

SnippetView::SnippetView(SnippetPlugin* plugin, QWidget* parent)
 : QWidget(parent), Ui::SnippetViewBase(), m_plugin(plugin)
{
    Ui::SnippetViewBase::setupUi(this);

    setWindowTitle(i18n("Snippets"));

    connect(filterText, SIGNAL(clearButtonClicked()),
            this, SLOT(slotFilterChanged()));
    connect(filterText, SIGNAL(textChanged(QString)),
            this, SLOT(slotFilterChanged()));
    connect(snippetTree, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotSnippetClicked(QModelIndex)));

    snippetTree->setContextMenuPolicy( Qt::CustomContextMenu );
    connect(snippetTree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(contextMenu(const QPoint&)));

    m_proxy = new SnippetFilterProxyModel(this);

    m_proxy->setSourceModel( SnippetStore::self() );

    snippetTree->setModel( m_proxy );
//     snippetTree->setModel( SnippetStore::instance() );

    snippetTree->header()->hide();

    m_addRepoAction = new KAction(KIcon("folder-new"), i18n("Add Repository"), this);
    connect(m_addRepoAction, SIGNAL(triggered()), this, SLOT(slotAddRepo()));
    addAction(m_addRepoAction);
    m_editRepoAction = new KAction(KIcon("folder-txt"), i18n("Edit Repository"), this);
    connect(m_editRepoAction, SIGNAL(triggered()), this, SLOT(slotEditRepo()));
    addAction(m_editRepoAction);
    m_removeRepoAction = new KAction(KIcon("edit-delete"), i18n("Remove Repository"), this);
    connect(m_removeRepoAction, SIGNAL(triggered()), this, SLOT(slotRemoveRepo()));
    addAction(m_removeRepoAction);

    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    addAction(separator);

    m_addSnippetAction = new KAction(KIcon("document-new"), i18n("Add Snippet"), this);
    connect(m_addSnippetAction, SIGNAL(triggered()), this, SLOT(slotAddSnippet()));
    addAction(m_addSnippetAction);
    m_editSnippetAction = new KAction(KIcon("document-edit"), i18n("Edit Snippet"), this);
    connect(m_editSnippetAction, SIGNAL(triggered()), this, SLOT(slotEditSnippet()));
    addAction(m_editSnippetAction);
    m_removeSnippetAction = new KAction(KIcon("document-close"), i18n("Remove Snippet"), this);
    connect(m_removeSnippetAction, SIGNAL(triggered()), this, SLOT(slotRemoveSnippet()));
    addAction(m_removeSnippetAction);

    connect(snippetTree->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(validateActions()));
    validateActions();
}

SnippetView::~SnippetView()
{
}

void SnippetView::validateActions()
{

    QStandardItem* item = currentItem();

    Snippet* selectedSnippet = dynamic_cast<Snippet*>( item );
    SnippetRepository* selectedRepo = dynamic_cast<SnippetRepository*>( item );

    m_addRepoAction->setEnabled(true);
    m_editRepoAction->setEnabled(selectedRepo);
    m_removeRepoAction->setEnabled(selectedRepo);

    m_addSnippetAction->setEnabled(selectedRepo || selectedSnippet);
    m_editSnippetAction->setEnabled(selectedSnippet);
    m_removeSnippetAction->setEnabled(selectedSnippet);
}

QStandardItem* SnippetView::currentItem()
{
    ///TODO: support multiple selected items
    QModelIndex index = snippetTree->currentIndex();
    index = m_proxy->mapToSource(index);
    return SnippetStore::self()->itemFromIndex( index );
}

void SnippetView::slotSnippetClicked (const QModelIndex & index)
{
    QStandardItem* item = SnippetStore::self()->itemFromIndex( m_proxy->mapToSource(index) );
    if (!item)
        return;

    Snippet* snippet = dynamic_cast<Snippet*>( item );
    if (!snippet)
        return;

    m_plugin->insertSnippet( snippet );
}

void SnippetView::contextMenu (const QPoint& pos)
{
    QModelIndex index = snippetTree->indexAt( pos );
    index = m_proxy->mapToSource(index);
    QStandardItem* item = SnippetStore::self()->itemFromIndex( index );
    if (!item) {
        // User clicked into an empty place of the tree
        KMenu menu(this);
        menu.addTitle(i18n("Snippets"));

        menu.addAction(m_addRepoAction);

        menu.exec(snippetTree->mapToGlobal(pos));
    } else if (Snippet* snippet = dynamic_cast<Snippet*>( item )) {
        KMenu menu(this);
        menu.addTitle(i18n("Snippet: %1", snippet->text()));

        menu.addAction(m_editSnippetAction);
        menu.addAction(m_removeSnippetAction);

        menu.exec(snippetTree->mapToGlobal(pos));
    } else if (SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item )) {
        KMenu menu(this);
        menu.addTitle(i18n("Repository: %1", repo->text()));

        menu.addAction(m_editRepoAction);
        menu.addAction(m_removeRepoAction);
        menu.addSeparator();

        menu.addAction(m_addSnippetAction);

        menu.exec(snippetTree->mapToGlobal(pos));
    }
}

void SnippetView::slotEditSnippet()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    Snippet* snippet = dynamic_cast<Snippet*>( item );
    if (!snippet)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item->parent() );
    if (!repo)
        return;

    EditSnippet dlg(repo, snippet, this);
    dlg.exec();
}

void SnippetView::slotAddSnippet()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item );
    if (!repo) {
        repo = dynamic_cast<SnippetRepository*>( item->parent() );
        if ( !repo ) 
            return;
    }

    EditSnippet dlg(repo, 0, this);
    dlg.exec();
}

void SnippetView::slotRemoveSnippet()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item->parent() );
    if (!repo)
        return;

    int ans = KMessageBox::warningContinueCancel(
        QApplication::activeWindow(),
        i18n("Do you really want to delete the snippet \"%1\"?", item->text())
    );
    if ( ans == KMessageBox::Continue ) {
        item->parent()->removeRow(item->row());
        repo->save();
    }
}

void SnippetView::slotAddRepo()
{
    EditRepository dlg(0, this);
    dlg.exec();
}

void SnippetView::slotEditRepo()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item );
    if (!repo)
        return;

    EditRepository dlg(repo, this);
    dlg.exec();
}

void SnippetView::slotRemoveRepo()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item );
    if (!repo)
        return;

    int ans = KMessageBox::warningContinueCancel(
        QApplication::activeWindow(),
        i18n("Do you really want to delete the repository \"%1\" with all its snippets?", repo->text())
    );
    if ( ans == KMessageBox::Continue ) {
        repo->remove();
    }
}

void SnippetView::slotFilterChanged()
{
    m_proxy->changeFilter( filterText->text() );
}

#include "snippetview.moc"
