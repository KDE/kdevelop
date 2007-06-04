/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetview.h"


#include <QHeaderView>
#include <QContextMenuEvent>
#include <QInputDialog>
#include <kmenu.h>

#include "snippet.h"
#include "snippetpart.h"
#include "snippetrepository.h"
#include "snippetstore.h"
#include "addrepository.h"
#include "editsnippet.h"
#include "snippetfilterproxymodel.h"
#include "moverepository.h"


SnippetView::SnippetView(SnippetPart* part, QWidget* parent)
 : QWidget(parent), Ui::SnippetViewBase(), part_(part)
{
    Ui::SnippetViewBase::setupUi(this);

    tbFilter->setIcon(KIcon("filter"));

    connect(snippetTree, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotSnippetClicked(QModelIndex)));
    connect(tbFilter, SIGNAL(clicked()),
            this, SLOT(slotFilterChanged()));

    snippetTree->setContextMenuPolicy( Qt::CustomContextMenu );
    connect(snippetTree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(contextMenu(const QPoint&)));

    proxy_ = new SnippetFilterProxyModel(this);

    proxy_->setSourceModel( SnippetStore::instance() );

    snippetTree->setModel( proxy_ );
//     snippetTree->setModel( SnippetStore::instance() );

    snippetTree->header()->hide();
}

SnippetView::~SnippetView()
{
    delete SnippetStore::instance();
}

QStandardItem* SnippetView::currentItem()
{
    QModelIndex index = snippetTree->currentIndex();
    index = proxy_->mapToSource(index);
    return SnippetStore::instance()->itemFromIndex( index );
}

void SnippetView::slotAddRepo()
{
    AddRepository dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        SnippetStore::instance()->createNewRepository(
                NULL, // create a new toplevel repository
                dlg.name->text(),
                dlg.location->text());
    }
}

void SnippetView::slotSnippetClicked (const QModelIndex & index)
{
    QStandardItem* item = SnippetStore::instance()->itemFromIndex( proxy_->mapToSource(index) );
    if (!item)
        return;

    Snippet* snippet = dynamic_cast<Snippet*>( item );
    if (!snippet)
        return;

    QString text = snippet->interpretSnippet();

    if (!text.isEmpty())
        part_->insertText( text );
}

void SnippetView::contextMenu (const QPoint& pos)
{
    QModelIndex index = snippetTree->indexAt( pos );
    index = proxy_->mapToSource(index);
    QStandardItem* item = SnippetStore::instance()->itemFromIndex( index );
    if (!item) {
        // User clicked into an empty place of the tree
        KMenu menu(this);
        menu.addTitle(i18n("Snippets"));

        QAction* add = menu.addAction(i18n("Add Repository"));
        connect(add, SIGNAL(triggered()), this, SLOT(slotAddRepo()));

        emit popupContextMenu(&menu, NULL);

        menu.exec(snippetTree->mapToGlobal(pos));
        return;
    }

    Snippet* snippet = dynamic_cast<Snippet*>( item );
    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item );

    if (snippet) {
        KMenu menu(this);
        menu.addTitle(i18n("Snippet")+ ": "+snippet->text());

        QAction* sync = menu.addAction(i18n("Sync"));
        connect(sync, SIGNAL(triggered()), snippet, SLOT(slotSyncSnippet()));

        QAction* edit = menu.addAction(i18n("Edit"));
        connect(edit, SIGNAL(triggered()), this, SLOT(slotEditSnippet()));

        QAction* del = menu.addAction(i18n("Delete"));
        connect(del, SIGNAL(triggered()), this, SLOT(slotDeleteSnippet()));

        emit popupContextMenu(&menu, item );

        menu.exec(snippetTree->mapToGlobal(pos));
    } else if (repo) {
        KMenu menu(this);
        menu.addTitle(i18n("Repository")+": "+repo->text());

        QAction* sync = menu.addAction(i18n("Sync"));
        connect(sync, SIGNAL(triggered()), repo, SLOT(slotSyncRepository()));

        QAction* add = menu.addAction(i18n("Add Snippet"));
        connect(add, SIGNAL(triggered()), this, SLOT(slotAddSnippet()));

        QAction* create = menu.addAction(i18n("Create Subrepository"));
        connect(create, SIGNAL(triggered()), this, SLOT(slotCreateSubRepo()));

        QAction* move = menu.addAction(i18n("Edit"));
        connect(move, SIGNAL(triggered()), this, SLOT(slotMoveRepository()));

        if (repo->QStandardItem::parent() == 0x0) {
            QAction* hide = menu.addAction(i18n("Hide"));
            connect(hide, SIGNAL(triggered()), this, SLOT(slotHideRepository()));
        }

        QAction* remove = menu.addAction(i18n("Remove"));
        connect(remove, SIGNAL(triggered()), this, SLOT(slotRemoveRepo()));

        emit popupContextMenu(&menu, item );

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

    EditSnippet dlg(snippet, this);
    dlg.exec();
}

void SnippetView::slotAddSnippet()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item );
    if (!repo)
        return;

    Snippet* snippet = new Snippet(i18n("New Snippet"), repo);
    EditSnippet dlg(snippet, this);
    if (dlg.exec() != QDialog::Accepted) {
        // the snippet file has not been written, so we need to remove this item from the model
        ///@todo maybe move this to somewhere else
        snippet->QStandardItem::parent()->removeRows( snippet->row(), 1 );
    }
}

void SnippetView::slotFilterChanged()
{
    proxy_->changeFilter( filterText->text() );
}

void SnippetView::slotDeleteSnippet()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    Snippet* snippet = dynamic_cast<Snippet*>( item );
    if (!snippet)
        return;

    snippet->removeSnippetFile();
}

void SnippetView::slotHideRepository()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item );
    if (!repo)
        return;

    SnippetStore::instance()->remove( repo );
}

void SnippetView::slotMoveRepository()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item );
    if (!repo)
        return;

    MoveRepository dlg(repo, this);
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

    repo->removeDirectory();
}

void SnippetView::slotCreateSubRepo()
{
    QStandardItem* item = currentItem();
    if (!item)
        return;

    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item );
    if (!repo)
        return;

    QString subdir = QInputDialog::getText(this, 
                            i18n("Create a new subrepository"), 
                            i18n("Directoryname"));
    if (!subdir.isEmpty()) {
        repo->createSubRepo( subdir );
    }
}

#include "snippetview.moc"
// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
