/***************************************************************************
                         kdlgitembase.cpp  -  
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


#include <qpixmap.h>
#include <qbitmap.h>
#include <klocale.h>
#include <kpopmenu.h>
#include <kruler.h>
#include <kiconloader.h>
#include <kcursor.h>
#include "../ckdevelop.h"
#include "kdlgitembase.h"
#include "kdlgpropertybase.h"
#include "kdlgeditwidget.h"
#include "kdlgpropwidget.h"
#include "kdlgproplv.h"
#include "itemsglobal.h"


KDlgItem_Base::KDlgItem_Base( KDlgEditWidget* editwid , QWidget *parent , bool ismainwidget, const char* name )
  : QObject(parent,name)
{
  editWidget = editwid;
  childs = 0;
  isMainwidget = ismainwidget;
  item = 0;
//  item = new QWidget(parent);
//  item->setMouseTracking(true);

  props = new KDlgPropertyBase();
  repaintItem();

  isItemActive = false;
  isMBPressed = false;
}


void KDlgItem_Base::recreateItem()
{
}


QString KDlgItem_Base::itemClass()
{
    return QString("[Base]");   
}


int KDlgItem_Base::getNrOfChilds()
{
    if (childs)
	return childs->numItems();
    else
	return 0;
}


bool KDlgItem_Base::addChild(KDlgItem_Base *itm)
{
    if (childs)
	return childs->addItem(itm);
    else
	return false;
}


void KDlgItem_Base::deleteMyself()
{
  if (!childs)
    return;

  KDlgItem_Base *w = childs->getFirst();

  while (w)
  {
    w->deleteMyself();
    if (w->getItem())
      delete w->getItem();
    delete w;
    w = childs->getNext();
  }

  childs->clear();
}


int KDlgItem_Base::Prop2Bool(QString name)
{
  if (props->getProp(name))
    return KDlgItemsIsValueTrue( props->getProp(name)->value );
  else
    return -1;
}


int KDlgItem_Base::Prop2Int(QString name, int defaultval)
{
  if (props->getProp(name))
    return props->getIntFromProp(name,defaultval);
  else
    return defaultval;
}


QString KDlgItem_Base::Prop2Str(QString name)
{
  if (props->getProp(name))
    return props->getProp(name)->value;
  else
    return QString();
}


void KDlgItem_Base::repaintItem(QWidget *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  int b=0;
  int x=0,y=0;
  QString str;

  #define Prop2Bool(name) (KDlgItemsIsValueTrue( props->getProp(name)->value ))
  #define Prop2Str(name) (props->getProp(name)->value)
  #define setB2Prop(name) b = Prop2Bool(name);
  #define setStr2Prop(name) str = Prop2Str(name);
  #define ifBValid(name) setB2Prop(name); if ((b==0) || (b==1))
  #define ifBTrue(name) setB2Prop(name); if (b==1)
  #define ifBFalse(name) setB2Prop(name); if (b==0)
  #define strNotEmpty(str) (str.length()!=0)

  itm->setMinimumWidth(props->getIntFromProp("MinWidth",0));
  itm->setMaximumWidth(props->getIntFromProp("MaxWidth",32767));
  itm->setMinimumHeight(props->getIntFromProp("MinHeight",0));
  itm->setMaximumHeight(props->getIntFromProp("MaxHeight",32767));

/*
  QString val = Prop2Str("BgMode");
  if (val.lower() == "fixedcolor")
    itm->setBackgroundMode(QWidget::FixedColor);
  else if (val.lower() == "fixedpixmap")
    itm->setBackgroundMode(QWidget::FixedPixmap);
  else if (val.lower() == "nobackground")
    itm->setBackgroundMode(QWidget::NoBackground);
  else if (val.lower() == "paletteforeground")
    itm->setBackgroundMode(QWidget::PaletteForeground);
  else if (val.lower() == "palettebackground")
    itm->setBackgroundMode(QWidget::PaletteBackground);
  else if (val.lower() == "palettelight")
    itm->setBackgroundMode(QWidget::PaletteLight);
  else if (val.lower() == "palettemidlight")
    itm->setBackgroundMode(QWidget::PaletteMidlight);
  else if (val.lower() == "palettedark")
    itm->setBackgroundMode(QWidget::PaletteDark);
  else if (val.lower() == "palettemid")
    itm->setBackgroundMode(QWidget::PaletteMid);
  else if (val.lower() == "palettetext")
    itm->setBackgroundMode(QWidget::PaletteText);
  else if (val.lower() == "palettebase")
    itm->setBackgroundMode(QWidget::PaletteBase);
  else
    itm->setBackgroundMode(QWidget::PaletteBackground);
*/

  ifBTrue("IsFixedSize")
    itm->setFixedSize( props->getIntFromProp("Width",itm->width()),
                       props->getIntFromProp("Height",itm->height()) );

  x=props->getIntFromProp("SizeIncX",1);
  y=props->getIntFromProp("SizeIncY",1);

  if ((x!=1) || (y!=1))
    itm->setSizeIncrement(x,y);


  itm->setGeometry(isMainwidget ? RULER_WIDTH : (props->getIntFromProp("X",itm->x())),
                   isMainwidget ? RULER_HEIGHT : (props->getIntFromProp("Y",itm->y())),
                   props->getIntFromProp("Width",itm->width()),
                   props->getIntFromProp("Height",itm->height()));

  ifBTrue("IsHidden")
    itm->hide();
  else
    itm->show();

//  ifBValid("IsEnabled")
//    itm->setEnabled(b);

  setStr2Prop("BgPixmap");
  if (strNotEmpty(str))
    itm->setBackgroundPixmap( QPixmap( str ) );
  else
    itm->setBackgroundPixmap( QPixmap() );

  setStr2Prop("MaskBitmap");
  if (strNotEmpty(str))
    itm->setMask( QBitmap( str ) );
  else
    itm->setMask( QBitmap() );

  setStr2Prop("Font");
  if (strNotEmpty(str))
    {
    itm->setFont(KDlgItemsGetFont(str));
//    itm->setFont(KDlgItemsGetFont("\"helvetica\" \"20\" \"75\" \"TRUE\""));
    }
  else
    {
      QWidget wid;
      itm->setFont(wid.font());
    }

}


void KDlgItem_Base::execContextMenu(bool ismain)
{
    KPopupMenu phelp;
    phelp.setTitle( itemClass() );
    if (!ismain)
        {
          phelp.insertItem( BarIcon("prev"), i18n("&Raise"),
			    getEditWidget(), SLOT(slot_raiseSelected()) );
          phelp.insertItem( BarIcon("next"), i18n("&Lower"),
			    getEditWidget(), SLOT(slot_lowerSelected()) );
          phelp.insertItem( BarIcon("top"), i18n("Raise to &top"),
			    getEditWidget(), SLOT(slot_raiseTopSelected()) );
          phelp.insertItem( BarIcon("bottom"), i18n("Lower to &bottom"),
			    getEditWidget(), SLOT(slot_lowerBottomSelected()) );
          phelp.insertSeparator();
          phelp.insertItem( BarIcon("cut"), i18n("C&ut"),
			    getEditWidget(), SLOT(slot_cutSelected()) );
          phelp.insertItem( BarIcon("delete"), i18n("&Delete"),
			    getEditWidget(), SLOT(slot_deleteSelected()) );
          phelp.insertItem( BarIcon("copy"), i18n("&Copy"),
			    getEditWidget(), SLOT(slot_copySelected()) );
        }
    phelp.insertItem( BarIcon("paste"), i18n("&Paste"),
		      getEditWidget(), SLOT(slot_pasteSelected()) );
    phelp.insertSeparator();
    phelp.insertItem( BarIcon("help"), i18n("&Help"),
		      getEditWidget(), SLOT(slot_helpSelected()) );
    phelp.exec(QCursor::pos());
}


void KDlgItem_Base::moveRulers(QWidget *widget, QMouseEvent *e )
{
    int gx = getEditWidget()->gridSizeX();
    int gy = getEditWidget()->gridSizeY();
    //    int x = e->pos().x()+widget->recPosX(0);
    //    int y = e->pos().y()+widget->recPosY(0);
    QPoint relpos = e->pos();
    relpos += widget->mapToGlobal(QPoint(0,0));
    relpos -= getEditWidget()->mapToGlobal(QPoint(0,0));
    int x = ((int)(relpos.x()/gx))*gx;
    int y = ((int)(relpos.y()/gy))*gy;
    getEditWidget()->horizontalRuler()->slotNewValue(x);
    getEditWidget()->verticalRuler()->slotNewValue(y);

    if (!isMBPressed)
        {
            QString xy = QString().sprintf(i18n("X:%.3d Y:%.3d "), x, y);
            getEditWidget()->getCKDevel()->kdlg_get_statusbar()->changeItem((const char*)xy, ID_KDLG_STATUS_XY);
            getEditWidget()->getCKDevel()->kdlg_get_statusbar()->changeItem("", ID_KDLG_STATUS_WH);
        }
}

#include <iostream.h>
bool KDlgItem_Base::eventFilter( QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Paint)
        {
            QWidget *widget = (QWidget *)o;
            QPaintEvent *pe = (QPaintEvent *)e;
            cout << "PaintEvent on " << widget->className() << endl;

            widget->removeEventFilter(this);
            qApp->sendEvent(widget, e);
            widget->installEventFilter(this);
            if (isItemActive)
                {
                    QPainter p(widget);
                    p.setClipRect(pe->rect());
                    KDlgItemsPaintRects(&p, widget->width(), widget->height());
                }
            return true;
        }
    if (e->type() == QEvent::MouseButtonPress)
        {
            QWidget *widget = (QWidget *)o;
            QMouseEvent *mpe = (QMouseEvent *)e;
            cout << "MousePressEvent on " << widget->className() << endl;
            
            getEditWidget()->selectWidget(this);
            
            if (mpe->button() == LeftButton)
                {
                    getEditWidget()->setModified(true);
                    isMBPressed = true;
                    startPnt = mpe->globalPos();
                    lastPnt = mpe->globalPos();
                    origRect = widget->geometry();
                    pressedEdge = KDlgItemsGetClickedRect(mpe->pos().x(), mpe->pos().y(),
                                                          widget->width(), widget->height());
                }
            
            if (mpe->button() == RightButton)
                {
                    execContextMenu(isMainwidget);
                }
            return true;
        }
    if (e->type() == QEvent::MouseButtonRelease)
        {
            QWidget *widget = (QWidget *)o;
            cout << "MouseReleaseEvent on " << widget->className() << endl;
            isMBPressed = false;
            KDlgPropWidget *pw = getEditWidget()->getCKDevel()->kdlg_get_prop_widget();
            if (pw)
                pw->refillList(this);
            return true;
        }
    if (e->type() == QEvent::MouseMove)
        {
            QWidget *widget = (QWidget *)o;
            QMouseEvent *mme = (QMouseEvent *)e;
            cout << "MouseMoveEvent on " << widget->className() << endl;
            
            moveRulers(widget, mme);
            if (isItemActive)
                {
                    int pE;
                    if (isMBPressed) 
                        pE = pressedEdge; 
                    else 
                        pE = KDlgItemsGetClickedRect(mme->pos().x(), mme->pos().y(), 
                                                     widget->width(), widget->height());
                    KDlgItemsSetMouseCursor(widget, pE);
                }
            else
                widget->setCursor(KCursor::arrowCursor());
            
            if ((!isMBPressed) || (mme->pos() == lastPnt)) return true;
            int gx = getEditWidget()->gridSizeX();
            int gy = getEditWidget()->gridSizeY();
            int x = origRect.x();
            int y = origRect.y();
            int w = origRect.width();
            int h = origRect.height();
            int diffx = mme->globalPos().x() - startPnt.x();
            int diffy = mme->globalPos().y() - startPnt.y();
            diffx = ((int)(diffx/gx))*gx;
            diffy = ((int)(diffy/gy))*gy;
            bool noMainWidget;
            noMainWidget = KDlgItemsGetResizeCoords(pressedEdge, x, y, w, h, diffx, diffy);
            
            if ((x!=origRect.x()) || (y!=origRect.y()))
                {
                    x = ((int)(x/gx))*gx;
                    y = ((int)(y/gy))*gy;
                }
            if ((w!=origRect.width()) || (h!=origRect.height()))
                {
                    w = ((int)(w/gx))*gx;
                    h = ((int)(h/gy))*gy;
                }
            if (!parentWidgetItem)
                {
                    if (x<0) x = 0;
                    if (y<0) y = 0;
                }
            if (w<10) w = 10;
            if (h<10) h = 10;
            
            if (isMBPressed)
                {
                    QString xy = QString().sprintf(i18n("X:%.3d Y:%.3d "), x, y);
                    QString wh = QString().sprintf(i18n("W:%.3d H:%.3d "), w, h);
                    getEditWidget()->getCKDevel()->kdlg_get_statusbar()->changeItem((const char*)xy, ID_KDLG_STATUS_XY);
                    getEditWidget()->getCKDevel()->kdlg_get_statusbar()->changeItem((const char*)wh, ID_KDLG_STATUS_WH);
                }
            
            if ((!noMainWidget) || (!isMainwidget))
                {
                    widget->setGeometry(x,y,w,h);
                    if (isMainwidget)
                        {
                            x = 0;
                            y = 0;
                        }
                    getProps()->setProp_Value("X",QString().setNum(x));
                    getProps()->setProp_Value("Y",QString().setNum(y));
                    getProps()->setProp_Value("Width",QString().setNum(w));
                    getProps()->setProp_Value("Height",QString().setNum(h));
                    if (isMainwidget && !noMainWidget)
                        {
                            getEditWidget()->verticalRuler()->setRange(0,h);
                            getEditWidget()->horizontalRuler()->setRange(0,w);
                        }
                    AdvListView *lv = getEditWidget()->getCKDevel()->kdlg_get_prop_widget()->getListView();
                    if (lv)
                        lv->setGeometryEntrys(x,y,w,h);
                }
            lastPnt = mme->pos();
            return true;
        }
    return false;
}


void KDlgItem_Base::select()
{
    if (!isItemActive)
	{
	    isItemActive = true;
	    getItem()->repaint();
	}
}


void KDlgItem_Base::deselect()
{
    if (isItemActive)
	{
	    isItemActive = false;
	    getItem()->repaint();
	}
}
