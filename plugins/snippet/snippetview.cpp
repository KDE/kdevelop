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
 : QWidget(parent), Ui::SnippetViewBase(), plugin_(plugin)
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

    proxy_ = new SnippetFilterProxyModel(this);

    proxy_->setSourceModel( SnippetStore::self() );

    snippetTree->setModel( proxy_ );
//     snippetTree->setModel( SnippetStore::instance() );

    snippetTree->header()->hide();

    m_addRepoAction = new KAction(KIcon("folder-new"), i18n("Add Repository"), this);
    connect(m_addRepoAction, SIGNAL(triggered()), this, SLOT(slotAddRepo()));
    addAction(m_addRepoAction);
    m_editRepoAction = new KAction(KIcon("folder-text"), i18n("Edit Repository"), this);
    connect(m_editRepoAction, SIGNAL(triggered()), this, SLOT(slotEditRepo()));
    addAction(m_editRepoAction);
    m_removeRepoAction = new KAction(KIcon("edit-delete"), i18n("Remove Repository"), this);
    connect(m_removeRepoAction, SIGNAL(triggered()), this, SLOT(slotRemoveRepo()));
    addAction(m_removeRepoAction);
    m_toggleRepoAction = new KAction(this);
    connect(m_toggleRepoAction, SIGNAL(triggered()), this, SLOT(slotToggleRepo()));
    addAction(m_toggleRepoAction);

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

    m_toggleRepoAction->setEnabled(selectedRepo);
    if ( selectedRepo ) {
        if ( selectedRepo->isEnabled() ) {
            m_toggleRepoAction->setIcon(KIcon("folder-red"));
            m_toggleRepoAction->setText(i18n("Disable Repository"));
            m_toggleRepoAction->setToolTip(i18n("Snippets from disabled repositories will not be shown during code completion."));
        } else {
            m_toggleRepoAction->setIcon(KIcon("folder-green"));
            m_toggleRepoAction->setText(i18n("Enable Repository"));
            m_toggleRepoAction->setToolTip(i18n("Snippets from enabled repositories will be shown during code completion."));
        }
    }

    m_addSnippetAction->setEnabled(selectedRepo || selectedSnippet);
    m_editSnippetAction->setEnabled(selectedSnippet);
    m_removeSnippetAction->setEnabled(selectedSnippet);
}

QStandardItem* SnippetView::currentItem()
{
    ///TODO: support multiple selected items
    QModelIndex index = snippetTree->currentIndex();
    index = proxy_->mapToSource(index);
    return SnippetStore::self()->itemFromIndex( index );
}

void SnippetView::slotSnippetClicked (const QModelIndex & index)
{
    QStandardItem* item = SnippetStore::self()->itemFromIndex( proxy_->mapToSource(index) );
    if (!item)
        return;

    Snippet* snippet = dynamic_cast<Snippet*>( item );
    if (!snippet)
        return;

    plugin_->insertSnippet( snippet );
}

void SnippetView::contextMenu (const QPoint& pos)
{
    QModelIndex index = snippetTree->indexAt( pos );
    index = proxy_->mapToSource(index);
    QStandardItem* item = SnippetStore::self()->itemFromIndex( index );
    if (!item) {
        // User clicked into an empty place of the tree
        KMenu menu(this);
        menu.addTitle(i18n("Snippets"));

        QAction* add = menu.addAction(i18n("Add Repository"));
        add->setIcon(KIcon("folder-new"));
        connect(add, SIGNAL(triggered()), this, SLOT(slotAddRepo()));

        menu.exec(snippetTree->mapToGlobal(pos));
    } else if (Snippet* snippet = dynamic_cast<Snippet*>( item )) {
        KMenu menu(this);
        menu.addTitle(i18n("Snippet: %1", snippet->text()));

        QAction* edit = menu.addAction(i18n("Edit"));
        edit->setIcon(KIcon("document-edit"));
        connect(edit, SIGNAL(triggered()), this, SLOT(slotEditSnippet()));

        QAction* del = menu.addAction(i18n("Delete"));
        del->setIcon(KIcon("document-close"));
        connect(del, SIGNAL(triggered()), this, SLOT(slotRemoveSnippet()));

        menu.exec(snippetTree->mapToGlobal(pos));
    } else if (SnippetRepository* repo = dynamic_cast<SnippetRepository*>( item )) {
        KMenu menu(this);
        menu.addTitle(i18n("Repository")+": "+repo->text());

        menu.addAction(m_toggleRepoAction);
        menu.addSeparator();

        QAction* edit = menu.addAction(i18n("Edit"));
        edit->setIcon(KIcon("folder-txt"));
        connect(edit, SIGNAL(triggered()), this, SLOT(slotEditRepo()));

        QAction* remove = menu.addAction(i18n("Remove"));
        remove->setIcon(KIcon("edit-delete"));
        connect(remove, SIGNAL(triggered()), this, SLOT(slotRemoveRepo()));

        menu.addSeparator();

        QAction* add = menu.addAction(i18n("Add Snippet"));
        add->setIcon(KIcon("document-new"));
        connect(add, SIGNAL(triggered()), this, SLOT(slotAddSnippet()));

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

void SnippetView::slotToggleRepo()
{
    SnippetRepository* repo = dynamic_cast<SnippetRepository*>( currentItem() );
    if ( !repo ) {
        return;
    }
    repo->setActive(!repo->isEnabled());
    validateActions();
}

void SnippetView::slotFilterChanged()
{
    proxy_->changeFilter( filterText->text() );
}

#include "snippetview.moc"
