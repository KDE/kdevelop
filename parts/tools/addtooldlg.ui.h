/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <q3header.h>
#include <kurlrequester.h>
#include "kapplicationtree.h"

void AddToolDialog::init()
{
    connect( tree, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(treeSelectionChanged(Q3ListViewItem*)) );
    tree->header()->hide();
}

QString AddToolDialog::getApp()
{
    return execEdit->url() + " " + paramEdit->text();
}

void AddToolDialog::treeSelectionChanged( Q3ListViewItem * lvi )
{
    KDevAppTreeListItem* item = dynamic_cast<KDevAppTreeListItem*> ( lvi );
    if ( !item || item->isDirectory() )
	return;
    execEdit->setURL( item->executable() );
    menutextEdit->setText( item->text( 0 ) );   
}
