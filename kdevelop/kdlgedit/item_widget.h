/***************************************************************************
                          item_widget.h  -  description
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


#ifndef ITEM_WIDGET_H
#define ITEM_WIDGET_H

#include <qframe.h>

#include "item_base.h"
#include "itemsglobal.h"

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */
	
#include "kdlgeditwidget.h"
#include <kruler.h>
#include "defines_item.h"



class KDlgItem_Widget : public KDlgItem_Base
{
  Q_OBJECT

  public:
    KDlgItem_Widget( KDlgEditWidget* editwid = 0, QWidget *parent = 0, bool ismainwidget = false, const char* name = 0 );

  MYITEMCLASS_BEGIN( QFrame )
    public:
      MyWidget(KDlgItem_Widget* wid, QWidget* parent = 0, bool isMainWidget = false, const char* name = 0);

    MYITEMCLASS_STDSTUFF( KDlgItem_Widget )
  MYITEMCLASS_END

  ITEMWRAPPER_STDSTUFF( KDlgItem_Widget, QWidget, "QWidget" )
};



#endif





