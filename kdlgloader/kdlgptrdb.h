/***************************************************************************
                          kdlgptrdb.h  -  description                              
                             -------------------                                         
    begin                : Fri Jun 4 1999                                           
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


#ifndef KDLGPTRDB_H
#define KDLGPTRDB_H

#include <qwidget.h>
#include <qlist.h>

/**Stores pointers to the dialog items used to give users the possiblity of reusing widgets.
  *@author Pascal Krahmer
  */

class KDlgPtrDb {
  public:
	  KDlgPtrDb(KDlgPtrDb *parent, QWidget* ch);
  	~KDlgPtrDb();

    void deleteMyself();
  	void addGroup(KDlgPtrDb* wid) { groups.append( wid ); }

  	void setName(QString n);
  	void setVarName(QString n);
  	
  	QWidget* getItemPtrVarName(QString itemName);
  	QWidget* getItemPtrName(QString itemName);
  	  	
    QWidget* child;
    QList<KDlgPtrDb> groups;

    QString Name;
    QString VarName;
    KDlgPtrDb *Parent;
};

#endif












