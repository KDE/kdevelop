/***************************************************************************
                          ctreeview.cc  -  implementation
                             -------------------
    begin                : Fri Mar 29 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "ctreeview.h"
#include "ctreehandler.h"

#include <kpopupmenu.h>

#include <kdebug.h>

#include <qheader.h>

#include <assert.h>
#include <iostream>
using namespace std;


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------- CTreeView::CTreeView()
 * CTreeView()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *   name           The name of this widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CTreeView::CTreeView(QWidget* parent /* = 0 */,const char* name /* = 0 */)
  : KListView (parent, name)
  , treeH( NULL )
{
  // Initialize the object.
  setRootIsDecorated( true );
  header()->hide();
  setSorting(-1,false);
  setFrameStyle( Panel | Sunken );
  setLineWidth( 2 );
  // remember, there is only this one, KDE conforming signal to connect
  connect (this, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
           SLOT(slotContextMenu (KListView *, QListViewItem *, const QPoint &)));
  // Initialize the treehandler.
  treeH = new CTreeHandler();
  treeH->setTree( this );

  addColumn( "clmn0" );
}

/*------------------------------------------ CTreeView::~CTreeView()
 * ~CTreeView()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CTreeView::~CTreeView()
{
  delete treeH; treeH = NULL;
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CTreeView::setTreeHandler()
 * setTreeHandler()
 *   Set a new treehandler.
 *
 * Parameters:
 *   aHandler        The new handler.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CTreeView::setTreeHandler( CTreeHandler *aHandler )
{
  assert( aHandler != NULL );

  delete treeH;

  treeH = aHandler;
  treeH->setTree( this );
}

/*------------------------------------- CTreeView::setTreeHandler()
 * setTreeHandler()
 *   Find an entry by using its' name.
 *
 * Parameters:
 *   aName          Name of the entry to find.
 *
 * Returns:
 *   QListViewItem  The item.
 *   NULL           No item was found.
 *-----------------------------------------------------------------*/
QListViewItem *CTreeView::findByName( const char * /*aName*/ )
{
  debug( "CTreeView::findByName NOT IMPLEMENTED YET!" );
  return NULL;
}

/*--------------------------------- CClassView::slotExecuted()
 * slotExecuted()
 *   Event when a user causes the KDE-2 conforming contextMenu() signal
 *   i.e. probably a right mouse click
 *-----------------------------------------------------------------*/
void CTreeView::slotContextMenu (KListView* listview, QListViewItem* item, const QPoint& pos)
{
  if (!listview||!item) return;
  kdDebug() << "in CTreeView::slotContextMenu(), item= " << item << "\n";
  // Set the sent item as active.
  setSelected( item, true );
  // Fetch the popupmenu for the current item and show it if it exists
  KPopupMenu* popup = getCurrentPopup();
  if( popup )
    popup->popup( pos );
}

/*-----------------------------------------------------------------*/
#include "ctreeview.moc"
