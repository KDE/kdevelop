/***************************************************************************
                          item_pushbutton.h  -  description
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


#ifndef ITEM_PUSHBUTTON_H
#define ITEM_PUSHBUTTON_H

#include <qpushbutton.h>
#include "item_widget.h"
#include "defines_item.h"

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */



class KDlgItem_PushButton : public KDlgItem_Base
{
  Q_OBJECT

  MYITEMCLASS_BEGIN( QPushButton )
    MYITEMCLASS_STDSTUFF( KDlgItem_PushButton )
    virtual void mousePressEvent ( QMouseEvent * ) { selectMe(); }
    virtual void mouseMoveEvent ( QMouseEvent *e ) { moveRulers(e); }
  MYITEMCLASS_END

  ITEMWRAPPER_STDSTUFF( KDlgItem_PushButton, QPushButton, "PushButton" )
};


#endif





