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
#include <assert.h>

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
  : QListView (parent, name)
{
  // Initialize the object.
  setRootIsDecorated( true );
  header()->hide();
  setSorting(-1,false);

  setFrameStyle( Panel | Sunken );
  setLineWidth( 2 );

  // Add callback for clicks in the listview.
  connect(this,
          SIGNAL(rightButtonPressed( QListViewItem *, const QPoint &, int)),
          SLOT(slotRightButtonPressed( QListViewItem *,const QPoint &,int)));

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
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
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

  treeH = aHandler;
  treeH->setTree( this );
}

/*********************************************************************
 *                                                                   *
 *                              EVENTS                               *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CTreeView::mousePressEvent()
 * mousePressEvent()
 *   Handles mousepressevents(duh!). If the left or right mouse 
 *   button is pressed the coordinate and the mousebutton is saved.
 *
 * Parameters:
 *   event           The event.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CTreeView::mousePressEvent(QMouseEvent * event)
{
  // Save the mousebutton.
  mouseBtn = event->button();

  if( mouseBtn == LeftButton || mouseBtn == RightButton )
    mousePos = event->pos();

  QListView::mousePressEvent( event );
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------- CClassView::slotRightButtonPressed()
 * slotRightButtonPressed()
 *   Event when a user selects someting in the tree.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CTreeView::slotRightButtonPressed(QListViewItem *item,
                                       const QPoint &p,int i)
{
  KPopupMenu *popup;

  if( item )
  {
    // Set the sent item as active.
    setSelected( item, true );

    // Fetch the popupmenu for the current item.
    popup = getCurrentPopup();

    // If the exists a popupmenu we show it.
    if( popup )
      popup->popup( this->mapToGlobal( mousePos ) );
  }
}
