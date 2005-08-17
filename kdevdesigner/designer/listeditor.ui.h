/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <q3header.h>
#include <qapplication.h>

void ListEditor::init()
{
    listview->header()->hide();
    listview->setSorting( -1 );
    listview->setDefaultRenameAction( Q3ListView::Accept );
}

void ListEditor::setList( const QStringList &l )
{
    Q3ListViewItem *i = 0;
    for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
	i = new Q3ListViewItem( listview, i );
	i->setText( 0, *it );
	i->setRenameEnabled( 0, TRUE );
    }
}

void ListEditor::addItem()
{
    Q3ListViewItem *i = new Q3ListViewItem( listview, listview->lastItem() );
    i->setRenameEnabled( 0, TRUE );
    qApp->processEvents();
    i->startRename( 0 );
}

void ListEditor::renamed( Q3ListViewItem *i )
{
    if ( i && i->text( 0 ).isEmpty() )
	i->startRename( 0 );
}

void ListEditor::removeItems()
{
    Q3ListViewItemIterator it( listview );
    Q3ListViewItem *i = 0;
    while ( ( i = it.current() ) ) {
	++it;
	if ( i->isSelected() )
	    delete i;
    }
}

QStringList ListEditor::items()
{
    QStringList l;
    Q3ListViewItemIterator it( listview );
    Q3ListViewItem *i = 0;
    while ( ( i = it.current() ) ) {
	++it;
	if ( !i->text( 0 ).isEmpty() )
	    l << i->text( 0 );
    }
    return l;
}

void ListEditor::renameItem()
{
    Q3ListViewItem *i = listview->currentItem();
    if ( i )
	i->startRename( 0 );
}
