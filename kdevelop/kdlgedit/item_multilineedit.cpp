/***************************************************************************
                     item_multilineedit.cpp  -  description
                             -------------------                                         
    begin                :  Tue May 18 1999                                           
    copyright            : (C) 1999 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#define DONTINC_ALL
#define INC_MULTILINEEDIT
#include "items.h"
#include "itemsglobal.h"

#define CALLER_ITEMCLASS_NAME KDlgItem_MultiLineEdit
#define ITEMCLASS_NAME KDlgItem_MultiLineEdit
#define ITEMCLASS_TYPE QMultiLineEdit

#include "item_all.cpp.inc"


ITEMCLASS_NAME::MyWidget::MyWidget(ITEMCLASS_NAME* wid, QWidget* parent, const char* name )
  : ITEMCLASS_TYPE(parent,name)
{
  parentObject = wid;
  isItemActive = false;
  isMBPressed = false;
  isMainwidget = false;
  setCursor(arrowCursor);
}

void ITEMCLASS_NAME::MyWidget::paintEvent ( QPaintEvent *e )
{
  ITEMCLASS_TYPE::paintEvent(e);

  if (isItemActive)
    KDlgItemsPaintRects(this,e);
}

ITEMCLASS_NAME::ITEMCLASS_NAME( KDlgEditWidget* editwid, QWidget *parent , const char* name )
  : KDlgItem_Base(editwid, parent,false,name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new MyWidget(this, parent);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
}

void ITEMCLASS_NAME::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void ITEMCLASS_NAME::addMyPropEntrys()
{
  if (!props)
    return;
  props->addProp("isAutoUpdate",   "true",             "General",        ALLOWED_BOOL);
  props->addProp("isReadOnly",   "false",             "General",        ALLOWED_BOOL);
  props->addProp("isOverWriteMode",   "false",             "General",        ALLOWED_BOOL);
  props->addProp("Text",           "",       "General",        ALLOWED_MULTISTRING);
  props->addProp("isTextSelected", "false",  "General",        ALLOWED_BOOL);
  props->addProp("setFixedVisibleLines",   "",             "Geometry",        ALLOWED_INT);
}

void ITEMCLASS_NAME::repaintItem(ITEMCLASS_TYPE *it)
{
  ITEMCLASS_TYPE *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
  #define strIsDef(s) (!Prop2Str(s).isNull())
  #define intIsDef(s) (!Prop2Str(s).isEmpty())

  itm->setAutoUpdate(Prop2Bool("isAutoUpdate") == 1 ? TRUE : FALSE);
  itm->setReadOnly(Prop2Bool("isReadOnly") == 1 ? TRUE : FALSE);
  itm->setOverwriteMode(Prop2Bool("isOverWriteMode") == 1 ? TRUE : FALSE);

  if (strIsDef("Text"))
  {
    int i = 0;
    QString src = Prop2Str("Text");
    QString s;
    QString dst;

    s = getLineOutOfString(src,i,"\\n");
    while (!s.isNull())
      {
        dst = dst + s + "\n";
        i++;
        s = getLineOutOfString(src,i,"\\n");
      }

    itm->setText( dst.left(dst.length()-1) );
  }

  if (Prop2Bool("isTextSelected") == 1)
    itm->selectAll();
  else
    itm->deselect();
  
  if(Prop2Str("setFixedVisibleLines") != ""){
    itm->setFixedVisibleLines(Prop2Int("setFixedVisibleLines"));
  }
}
