/***************************************************************************
                 itemsglobal.h  -  helper file for item_*.* files
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef ITEMSGLOBAL_H
#define ITEMSGLOBAL_H

#include <qobject.h>
#include <qframe.h>

#include "defines.h"

class KDlgItem_Base;
class QFont;
class QColor;

QString KDlgLimitLines(QString src, unsigned maxlen = 40);

/**
 * Paints the border and the rectangles of a selected item.
*/
void KDlgItemsPaintRects(QPainter *p, int w, int h);

QFont KDlgItemsGetFont(QString desc);

QString getLineOutOfString(QString src, int ln, QString sep = "\n");

QColor Str2Color(QString desc);

/**
 * Paints the border and the rectangles of a selected item.
*/
void KDlgItemsPaintRects(QWidget *wid, QPaintEvent *e);

/**
 * Returns the constant of the clicked point (see RESIZE_* constants in defines.h)
 * @param winw width of the item
 * @param winh height of the item
 * @param x clicked x position in the item
 * @param y clicked y position in the item
*/
int  KDlgItemsGetClickedRect(int x, int y, int winw, int winh);

/**
 * Returns the new coordinates of the item.
 * @param pressedEdge RESIZE_* constant
 * @param diffx x difference between starting and recent point of the mouse cursor.
 * @param diffy y difference between starting and recent point of the mouse cursor.
*/
bool KDlgItemsGetResizeCoords(int pressedEdge, int &x, int &y, int &w, int &h, int diffx, int diffy);

/**
 * Sets the right mousecursor depending on what edge it is above.
*/
void KDlgItemsSetMouseCursor(QWidget* caller, int pressedEdge);

/**
 * Returns 1 if val == "TRUE", "1" or "YES"
 * Returns 0 if val == "FALSE", "0" or "NO"
 * Returns -1 if not 1 or 0 :-)
*/
int KDlgItemsIsValueTrue(QString val);


/**
 * Contains up to MAX_WIDGETS_PER_DIALOG items.
*/
class KDlgItemDatabase
{
  public:
    /**
     * What do you think a constructor is good for ? :-)
    */
    KDlgItemDatabase();
    ~KDlgItemDatabase();

    /**
     * Returns the items count.
    */
    int numItems();

    /**
     * Adds an Item to the database
    */
    bool addItem(KDlgItem_Base *item);

    /**
     * removes <i>item</i> from the database
    */
    void removeItem(KDlgItem_Base *item, bool deleteIt=false);

    /**
     * removes all items from the database
    */
    void clear() { int i; for (i=0; i<MAX_WIDGETS_PER_DIALOG; i++) items[i]=0; }

    /**
     * returns the first item.
    */
    KDlgItem_Base *getFirst() { recentGetNr = -1; return getNext(); }

    /**
     * returns the next item (run getFirst() first!)
    */
    KDlgItem_Base *getNext();

    int raiseItem(KDlgItem_Base*);
    int lowerItem(KDlgItem_Base*);

  protected:
    KDlgItem_Base *items[MAX_WIDGETS_PER_DIALOG];
    int recentGetNr;
};




#endif
