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


#ifndef KDLG_DEFINES_ITEM_H
#define KDLG_DEFINES_ITEM_H

#define MYITEMCLASS_BEGIN(mytype) protected: class MyWidget : public mytype {
#define MYITEMCLASS_END };

#define MYITEMCLASS_STDSTUFF(wrappertype) \
 public: \
   MyWidget(wrappertype* wid, QWidget* parent = 0, const char* name = 0); \
   int recPosX(int px) { int xnew = px+x(); if ((!parent()) || (parent()!=parentObject->getEditWidget())) return ((KDlgItem_Widget::MyWidget*)parent())->recPosX(xnew); else return px; } \
   int recPosY(int py) { int ynew = py+y(); if ((!parent()) || (parent()!=parentObject->getEditWidget())) return ((KDlgItem_Widget::MyWidget*)parent())->recPosY(ynew); else return py; } \
   bool isItemActive; \
   void deselect() { if (isItemActive) { isItemActive = false; repaint(); } } \
   void select() { if (!isItemActive) { isItemActive = true; repaint(); } } \
   void selectMe() { parentObject->getEditWidget()->selectWidget((KDlgItem_Base*)parentObject); } \
 protected: \
   wrappertype* parentObject; \
   void moveRulers( QMouseEvent *e ) { if (!parentObject) return; parentObject->getEditWidget()->horizontalRuler()->setValue(e->pos().x()+recPosX(0));parentObject->getEditWidget()->verticalRuler()->setValue(e->pos().y()+recPosY(0)); } \
   virtual void paintEvent ( QPaintEvent * );



#define ITEMWRAPPER_STDSTUFF(classname, widgettype, classdesc) \
  protected: \
    MyWidget *item; \
  public: \
    classname( KDlgEditWidget* editwid = 0, QWidget *parent = 0, const char* name = 0 ); \
    virtual ~classname() {} \
    virtual QString itemClass() { return QString(classdesc); } \
    virtual MyWidget* getItem() { return item; } \
    virtual void repaintItem(widgettype *it = 0); \
    virtual void select() { if (item) item->select(); } \
    virtual void selectMe() { if (item) item->selectMe(); } \
    virtual void deselect() { if (item) item->deselect(); }


#endif





