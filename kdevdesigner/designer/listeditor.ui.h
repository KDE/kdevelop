/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qheader.h>
#include <qapplication.h>

void ListEditor::init()
{
    listview->header()->hide();
    listview->setSorting( -1 );
    listview->setDefaultRenameAction( QListView::Accept );
}

void ListEditor::setList( const QStringList &l )
{
    QListViewItem *i = 0;
    for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
	i = new QListViewItem( listview, i );
	i->setText( 0, *it );
	i->setRenameEnabled( 0, TRUE );
    }
}

void ListEditor::addItem()
{
    QListViewItem *i = new QListViewItem( listview, listview->lastItem() );
    i->setRenameEnabled( 0, TRUE );
    qApp->processEvents();
    i->startRename( 0 );
}

void ListEditor::renamed( QListViewItem *i )
{
    if ( i && i->text( 0 ).isEmpty() )
	i->startRename( 0 );
}

void ListEditor::removeItems()
{
    QListViewItemIterator it( listview );
    QListViewItem *i = 0;
    while ( ( i = it.current() ) ) {
	++it;
	if ( i->isSelected() )
	    delete i;
    }
}

QStringList ListEditor::items()
{
    QStringList l;
    QListViewItemIterator it( listview );
    QListViewItem *i = 0;
    while ( ( i = it.current() ) ) {
	++it;
	if ( !i->text( 0 ).isEmpty() )
	    l << i->text( 0 );
    }
    return l;
}

void ListEditor::renameItem()
{
    QListViewItem *i = listview->currentItem();
    if ( i )
	i->startRename( 0 );
}
