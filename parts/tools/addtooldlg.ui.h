/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "treeview.h"

void AddToolDialog::init()
{
    connect( tree, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(treeSelectionChanged(QListViewItem*)) );
}

QString AddToolDialog::getApp()
{
    return execEdit->text() + " " + paramEdit->text();
}

void AddToolDialog::treeSelectionChanged( QListViewItem * lvi )
{
    TreeItem* item = dynamic_cast<TreeItem*> ( lvi );
    if ( !item )
	return;
    execEdit->setText( item->exec() );
    menutextEdit->setText( item->caption() );   
}
