/***************************************************************************
                 kdlgpropertybase.h  -  
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


#ifndef _KDLGPROPERTYBASE_H_
#define _KDLGPROPERTYBASE_H_

#include <qobject.h>
#include <qframe.h>
#include <qtextstream.h>
#include "defines.h"


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

  /**
   * can store some property-specific data.
  */
  QString data;
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
    int getEntryCount() const { return numEntrys; }

    /**
     * returns a pointer to the KDlgPropertyEntry (see above) given by <i>nr</i>
    */
    KDlgPropertyEntry* getProp(int nr);

    /**
     * returns a pointer to the KDlgPropertyEntry (see above) given by <i>name</i>.
     * @param name The name of the property (i.e. "Width").
    */
    KDlgPropertyEntry* getProp(QString name);
    /**
     * return the value of a property by a given prop name
     */
    QString getPropValue(QString name);
    bool propValueAsBool(QString name);

    void dumpConstruct(QTextStream *stream, QString classname,
		       QString parentname);
    void dumpPropCall(QTextStream *stream, QString methname,
		      QString contents);
    void dumpStringPropCall(QTextStream *stream, QString methname,
			    QString name, bool withi18n=false);
    void dumpIntPropCall(QTextStream *stream, QString methname,
			 QString name);
    void dumpBoolPropCall(QTextStream *stream, QString methname,
			  QString name, bool iffalse);
    void dumpColorPropCall(QTextStream *stream, QString methname,
			   QString name);
    void dumpPixmapPropCall(QTextStream *stream, QString methname,
			    QString name);	
    /**
     * sets all entries of a propertyentry in one step.
    */
    void setProp(int nr, QString name, QString value, QString group, int allowed, QString data = "")
       { if ((nr<0) || (nr>numEntrys)) return; props[nr].name = name; props[nr].value = value; props[nr].group = group; props[nr].allowed = allowed; props[nr].data = data; }

    void setProp_Name   (int nr, QString name)  { if ((nr<0) || (nr>numEntrys)) return; props[nr].name = name; }
    void setProp_Value  (int nr, QString value) { if ((nr<0) || (nr>numEntrys)) return; props[nr].value = value; }
    void setProp_Group  (int nr, QString group) { if ((nr<0) || (nr>numEntrys)) return; props[nr].group = group; }
    void setProp_Allowed(int nr, int allowed)   { if ((nr<0) || (nr>numEntrys)) return; props[nr].allowed = allowed; }
    void setProp_Data   (int nr, QString data)  { if ((nr<0) || (nr>numEntrys)) return; props[nr].data = data; }

    void setProp_Name   (QString n, QString name);
    /**
     * changes the value of property given by <i>n</i>(ame)
    */
    void setProp_Value  (QString n, QString value);
    void setProp_Group  (QString n, QString group);
    void setProp_Allowed(QString n, int allowed);
    void setProp_Data   (QString n, QString data);


    /**
     * Adds a property to the end of the list.
    */
    void addProp(QString name, QString value, QString group, int allowed, QString data = "" )
       { if (numEntrys>=MAX_ENTRYS_PER_WIDGET) return; setProp(++numEntrys, name, value, group, allowed, data); }

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
