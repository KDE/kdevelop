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

QString KDlgLimitLines(QString src, unsigned maxlen);

QFont KDlgItemsGetFont(QString desc);

QString getLineOutOfString(QString src, int ln, QString sep = "\n");

QColor Str2Color(QString desc);

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
    void removeItem(KDlgItem_Base *item);

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
