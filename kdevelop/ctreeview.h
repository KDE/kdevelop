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

#include <klistview.h>

class CProject;
class CTreeHandler;
class KPopupMenu;
class QListViewItem;

/** */
class CTreeView : public KListView
{
  Q_OBJECT

public: // Constructor & Destructor

  CTreeView( QWidget*parent=0,const char *name=0 );
  ~CTreeView();

public: // Public attributes

  /** Tells which(if any) of the buttons was pressed,
      (rokrau: this needs to be thrown out, 06/18/01) */
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

  /** The position at the last mousepress-event.
      (rokrau: this needs to be thrown out, 06/18/01) */
  QPoint mousePos;

protected: // Protected methods

  /** Get the current popupmenu. */
  virtual KPopupMenu *getCurrentPopup() = 0;

protected slots: // Protected slots
  /** KDE conforming signal contextMenu() connects to this slot
      Note, that the signal executed() has to be connected in each derived class,
      whereas signal contextMenu() can be handled in a generic way */
  void slotContextMenu (KListView* listview, QListViewItem* item, const QPoint& pos);
};

#endif
