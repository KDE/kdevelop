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

/**
 * Paints the border and the rectangles of a selected item.
*/
void KDlgItemsPaintRects(QPainter *p, int w, int h);

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

  protected:
    KDlgItem_Base *items[MAX_WIDGETS_PER_DIALOG];
    int recentGetNr;
};



/**
 * This is one property of a widget
*/
struct KDlgPropertyEntry
{
  /**
   * name of the property (i.e. "Width")
  */
  QString name;

  /**
   * value of the property (i.e. ¨TRUE")
  */
  QString value;

  /**
   * group the property consists to. (i.e. "Gerneral" or "Appearance")
  */
  QString group;

  /**
   * the allowed values for this property (see ALLOWED_* constants in defines.h)
  */
  int allowed;
};


/**
 * Contains all properties of one item.
*/
class KDlgPropertyBase
{
  public:
    /**
     * if fillWithStdEntrys is false no standard properties otherwise all
     * properties for a QWidget are added.
    */
    KDlgPropertyBase(bool fillWithStdEntrys = true);
    virtual ~KDlgPropertyBase() { }

    /**
     * adds all properties of an QWidget.
    */
    virtual void fillWithStandardEntrys();

    /**
     * returns the number of properties for the widget.
    */
    int getEntryCount() { return numEntrys; }

    #define testIfNrIsValid0(nr) if ((nr<0) || (nr>numEntrys)) return 0;
    #define testIfNrIsValid(nr) if ((nr<0) || (nr>numEntrys)) return;

    /**
     * returns a pointer to the KDlgPropertyEntry (see above) given by <i>nr</i>
    */
    KDlgPropertyEntry* getProp(int nr) { testIfNrIsValid0(nr); return &props[nr];  }

    /**
     * returns a pointer to the KDlgPropertyEntry (see above) given by <i>name</i>.
     * @param name The name of the property (i.e. "Width").
    */
    KDlgPropertyEntry* getProp(QString name);

    /**
     * sets all entries of a propertyentry in one step.
    */
    void setProp(int nr, QString name, QString value, QString group, int allowed)
       { testIfNrIsValid(nr); props[nr].name = name; props[nr].value = value; props[nr].group = group; props[nr].allowed = allowed; }

    void setProp_Name   (int nr, QString name)  { testIfNrIsValid(nr); props[nr].name = name; }
    void setProp_Value  (int nr, QString value) { testIfNrIsValid(nr); props[nr].value = value; }
    void setProp_Group  (int nr, QString group) { testIfNrIsValid(nr); props[nr].group = group; }
    void setProp_Allowed(int nr, int allowed)   { testIfNrIsValid(nr); props[nr].allowed = allowed; }

    void setProp_Name   (QString n, QString name);
    /**
     * changes the value of property given by <i>n</i>(ame)
    */
    void setProp_Value  (QString n, QString value);
    void setProp_Group  (QString n, QString group);
    void setProp_Allowed(QString n, int allowed);


    /**
     * Adds a property to the end of the list.
    */
    void addProp(QString name, QString value, QString group, int allowed )
       { if (numEntrys>=MAX_ENTRYS_PER_WIDGET) return; setProp(++numEntrys, name, value, group, allowed); }

    /**
     * Returns an integer of the value field of a property. If there is no valid integer is entered it returns <i>defaultval</i>.
    */
    int getIntFromProp(int nr, int defaultval=0);
    int getIntFromProp(QString name, int defaultval=0);

  protected:
    KDlgPropertyEntry props[MAX_ENTRYS_PER_WIDGET];
    int numEntrys;
};


#endif
