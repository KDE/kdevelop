/***************************************************************************
                          ctreeview.h  -  description
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

#ifndef _CTREEVIEW_INCLUDED
#define _CTREEVIEW_INCLUDED

#include <qlistview.h>
#include <qheader.h>
#include <kpopmenu.h>
#include <qpoint.h> 
#include "ctreehandler.h"

//#include "cproject.h"
class CProject;

/** */
class CTreeView : public QListView
{
  Q_OBJECT

public: // Constructor & Destructor

  CTreeView( QWidget*parent=0,const char *name=0 );
  ~CTreeView();

public: // Public attributes

  /** Tells which(if any) of the buttons where pressed. */
  int mouseBtn;

  /** The utilityclass to draw the tree. */
  CTreeHandler *treeH;

public: // Public methods

  /** Starts the refresh. */
  virtual void refresh(CProject* prj) = 0;

  /** Set a new treehandler. */
  void setTreeHandler( CTreeHandler *aHandler );

  /** Find an entry by using its' name. */
  QListViewItem *findByName( const char *aName );

protected: // Protected attributes

  /** The position at the last mousepress-event. */
  QPoint mousePos;

protected: // Protected methods

  /** Event to be executed on a mousepress. */
  void mousePressEvent(QMouseEvent* event);

  /** Initialize popupmenus. */
    //  virtual void initPopups() = 0;

  /** Get the current popupmenu. */
  virtual KPopupMenu *getCurrentPopup() = 0;

  protected slots: // Protected slots 
    void slotRightButtonPressed(QListViewItem *,const QPoint &,int);
};

#endif
