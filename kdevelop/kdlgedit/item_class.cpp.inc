/***************************************************************************
                          defines_item.h  -  description
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


#ifdef WIDGETTYPE
#ifdef CLASSNAME
#ifdef CLASSDESC

public:
    CLASSNAME(KDlgEditWidget *editwid, KDlgItem_Base *parentit);
    virtual ~CLASSNAME() {}
    virtual void repaintItem();
    virtual void addMyPropEntrys();

#undef CLASSDESC
#endif
#undef CLASSNAME
#endif
#undef WIDGETTYPE
#endif
