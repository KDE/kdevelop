/***************************************************************************
                         kdlgitems.h  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
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


#ifndef KDLGITEMS_H
#define KDLGITEMS_H

#include <qwidget.h>

class KTreeList;

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */

class KDlgItems : public QWidget  {
  Q_OBJECT
  public:
    KDlgItems(QWidget *parent=0, const char *name=0);
    ~KDlgItems();

    KTreeList *getTreeList() { return treelist; }
  protected:
    virtual void resizeEvent ( QResizeEvent * );

    KTreeList *treelist;
};

#endif
