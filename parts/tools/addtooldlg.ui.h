/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qheader.h>
#include <kurlrequester.h>
#include "kapplicationtree.h"

void AddToolDialog::init()
{
    connect( tree, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(treeSelectionChanged(QListViewItem*)) );
    tree->header()->hide();
}

QString AddToolDialog::getApp()
{
    return execEdit->url() + " " + paramEdit->text();
}

void AddToolDialog::treeSelectionChanged( QListViewItem * lvi )
{
    KDevAppTreeListItem* item = dynamic_cast<KDevAppTreeListItem*> ( lvi );
    if ( !item || item->isDirectory() )
	return;
    execEdit->setURL( item->executable() );
    menutextEdit->setText( item->text( 0 ) );   
}
