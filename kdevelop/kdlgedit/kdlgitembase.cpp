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
#include <qobjectlist.h>
#include <kapp.h>
#include <klocale.h>
#include <kpopmenu.h>
#include <kruler.h>
#include <kiconloader.h>
#include <kcursor.h>
#include "kdlgedit.h"
#include "kdlgitembase.h"
#include "kdlgpropertybase.h"
#include "kdlgeditwidget.h"
#include "kdlgpropwidget.h"
#include "kdlgproplv.h"
#include "itemsglobal.h"


KDlgItem_Base::KDlgItem_Base(KDlgEditWidget *editWidget, KDlgItem_Base *parentItem, Role role)
    : QObject(editWidget, "KDlgEdit item")
{
    editwid = editWidget;
    parentit = parentItem;
    rol = role;
    childs = 0;
    wid = 0;

  props = new KDlgPropertyBase();
  repaintItem();

  inPaintEvent = false;
  isItemActive = false;
  isMBPressed = false;
}


void KDlgItem_Base::setWidget(QWidget *widget)
{
    wid = widget;
    widget->installEventFilter(this);
    widget->setMouseTracking(true);
    if (widget->children()) {
        QListIterator<QObject> it(*widget->children());
        for (; it.current(); ++it)
            if (it.current()->isWidgetType()) {
                ((QWidget*)it.current())->installEventFilter(this);
                ((QWidget*)it.current())->setMouseTracking(true);
            }
    }
}


void KDlgItem_Base::recreateItem()
{
    widget()->recreate(widget()->parentWidget(), 0, widget()->pos(), true);

    if (childs) {
        KDlgItem_Base *it = childs->getFirst();
        while (it) {
            it->recreateItem();
            it = childs->getNext();
        }
    }
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
    if (w->widget())
      delete w->widget();
    delete w;
    w = childs->getNext();
  }

  childs->clear();
}

int KDlgItem_Base::Prop2Bool(QString name)
{
  if (props->getProp(name))
    return props->propValueAsBool(name);
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

void KDlgItem_Base::repaintItem()
{
  QWidget *itm = widget();

  if ((!itm) || (!props))
    return;

  int x=0,y=0;
  QString str;

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

  if (props->propValueAsBool("IsFixedSize"))
    itm->setFixedSize( props->getIntFromProp("Width",itm->width()),
                       props->getIntFromProp("Height",itm->height()) );

  x=props->getIntFromProp("SizeIncX",1);
  y=props->getIntFromProp("SizeIncY",1);

  if ((x!=1) || (y!=1))
    itm->setSizeIncrement(x,y);


  itm->setGeometry((role()==Main) ? RULER_WIDTH : (props->getIntFromProp("X",itm->x())),
                   (role()==Main) ? RULER_HEIGHT : (props->getIntFromProp("Y",itm->y())),
                   props->getIntFromProp("Width",itm->width()),
                   props->getIntFromProp("Height",itm->height()));

  if (KDlgItemsIsValueTrue(props->getProp("IsHidden")->value) == 1)
    itm->hide();
  else
    itm->show();

  str = props->getProp("BgPixmap")->value;
  if (!str.isEmpty())
    itm->setBackgroundPixmap( QPixmap( str ) );
  else
    itm->setBackgroundPixmap( QPixmap() );

  str = props->getProp("MaskBitmap")->value;
  if (!str.isEmpty())
    itm->setMask( QBitmap( str ) );
  else
    itm->setMask( QBitmap() );

  str = props->getProp("Font")->value;
  if (!str.isEmpty())
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


void KDlgItem_Base::execContextMenu()
{
    KPopupMenu phelp;
    phelp.setTitle( itemClass() );
    if (!role() == Main)
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


const int CornerSize = 8;

KDlgItem_Base::Corner KDlgItem_Base::cornerForPos(QPoint pos)
{
    int w = widget()->width();
    int h = widget()->height();
    int x = pos.x();
    int y = pos.y();
    
    if ( (x>=w-CornerSize) && (y>=h-CornerSize) && (x<=w) && (y<=h) )
        return BottomRight;
    if ( (x>=0) && (y>=0) && (x<=CornerSize) && (y<=CornerSize) )
        return TopLeft;
    if ( (x>=w-CornerSize) && (y>=0) && (x<=w) && (y<=CornerSize) )
        return TopRight;
    if ( (x>=0) && (y>=h-CornerSize) && (x<=CornerSize) && (y<=h) )
        return BottomLeft;
    if ( (x>=w/2-CornerSize/2) && (y>=0) && (x<=w/2+CornerSize/2) && (y<=CornerSize) )
        return MiddleTop;
    if ( (x>=w/2-CornerSize/2) && (y>=h-CornerSize) && (x<=w/2+CornerSize/2) && (y<=h) )
        return MiddleBottom;
    if ( (x>=0) && (y>=h/2-CornerSize/2) && (x<=CornerSize) && (y<=h/2+CornerSize/2) )
        return MiddleLeft;
    if ( (x>=w-CornerSize) && (y>=h/2-CornerSize/2) && (x<=w) && (y<=h/2+CornerSize/2) )
        return MiddleRight;
    
    return NoCorner;
}


bool KDlgItem_Base::getResizeCoords(Corner c, int diffx, int diffy,
                                    int *x, int *y, int *w, int *h)
{
    bool noMainWidget = false;
    
    switch (c)
        {
        case BottomRight:
            *w += diffx;
            *h += diffy;
            break;
        case MiddleRight:
            *w += diffx;
            break;
        case MiddleBottom:
            *h += diffy;
        break;
        case TopLeft:
            noMainWidget = true;
            if (*x + diffx < *x + *w-1) *x += diffx; else *x += *w-1;
            if (*y + diffy < *y + *h-1) *y += diffy; else *y += *h-1;
            *w -= diffx;
            *h -= diffy;
            break;
        case TopRight:
            noMainWidget = true;
            if (*y+diffy < *y + *h-1) *y += diffy; else *y += *h-1;
            *w += diffx;
            *h -= diffy;
            break;
        case MiddleLeft:
            noMainWidget = true;
            if (*x+diffx < *x + *w-1) *x += diffx; else *x += *w-1;
            *w -= diffx;
            break;
        case MiddleTop:
            noMainWidget = true;
            if (*y+diffy < *y + *h-1) *y += diffy; else *y += *h-1;
            *h -= diffy;
            break;
        case BottomLeft:
            noMainWidget = true;
            if (*x + diffx < *x + *w-1) *x += diffx; else *x += *w-1;
            *w -= diffx;
            *h += diffy;
            break;
        default:
            noMainWidget = true;
            *x += diffx;
            *y += diffy;
    }
    return noMainWidget;
}


void KDlgItem_Base::paintCorners(QPainter *p)
{
    int w = widget()->width();
    int h = widget()->height();

    QBrush b(Qt::Dense4Pattern);
    
    p->drawWinFocusRect(0,0,w,h);
    p->drawWinFocusRect(1,1,w-2,h-2);
    p->fillRect(0, 0, CornerSize, CornerSize, b);
    p->fillRect(w-CornerSize, 0, CornerSize, CornerSize, b);
    p->fillRect(0, h-CornerSize, CornerSize, CornerSize, b);
    p->fillRect(w-CornerSize, h-CornerSize, CornerSize, CornerSize, b);
    
    p->fillRect(w/2-CornerSize/2, 0, CornerSize, CornerSize, b);
    p->fillRect(w/2-CornerSize/2, h-CornerSize, CornerSize, CornerSize, b);
    
    p->fillRect(0, h/2-CornerSize/2, CornerSize, CornerSize, b);
    p->fillRect(w-CornerSize, h/2-CornerSize/2, CornerSize, CornerSize, b);
}


void KDlgItem_Base::paintGrid(QPainter *p)
{
    int w = widget()->width();
    int h = widget()->height();
    int gx = getEditWidget()->gridSizeX();
    int gy = getEditWidget()->gridSizeY();
    
    if ((gx<=1) || (gy<=1)) {
        if ((gx>1) || (gy>1)) {
            QPen oldpen = p->pen();
            QPen newpen(QColor(255,128,128), 0, DashDotLine);
            p->setPen(newpen);
            if (gx <= 1)
                for (int y = 0; y < h; y+=gy)
                    p->drawLine(3, y, w-6, y);
            else
                for (int x = 0; x < w; x+=gx)
                    p->drawLine(x, 3, x, h-6);
            p->setPen(oldpen);
        }
    } else {
        for (int x = 0; x < w; x+=gx)
            for (int y = 0; y < h; y+=gy)
                p->drawPoint(x, y);
    }
}


void KDlgItem_Base::setMouseCursorToEdge(Corner c)
{
    QCursor cursor;
    switch (c)
        {
        case BottomRight:
            cursor = KCursor::sizeFDiagCursor();
            break;
        case TopLeft:
            cursor = KCursor::sizeFDiagCursor();
            break;
        case TopRight:
            cursor = KCursor::sizeBDiagCursor();
            break;
        case BottomLeft:
            cursor = KCursor::sizeBDiagCursor();
            break;
        case MiddleTop:
            cursor = KCursor::sizeVerCursor();
            break;
        case MiddleBottom:
            cursor = KCursor::sizeVerCursor();
            break;
        case MiddleLeft:
            cursor = KCursor::sizeHorCursor();
            break;
        case MiddleRight:
            cursor = KCursor::sizeHorCursor();
            break;
        default:
            cursor = KCursor::arrowCursor();
        }
    widget()->setCursor(cursor);
}


void KDlgItem_Base::moveRulers(QPoint relpos)
{
    int gx = getEditWidget()->gridSizeX();
    int gy = getEditWidget()->gridSizeY();
    //    QPoint relpos = e->pos();
    relpos += widget()->mapToGlobal(QPoint(0,0));
    relpos -= getEditWidget()->mapToGlobal(QPoint(0,0));
    int x = ((int)(relpos.x()/gx))*gx;
    int y = ((int)(relpos.y()/gy))*gy;
    getEditWidget()->horizontalRuler()->slotNewValue(x);
    getEditWidget()->verticalRuler()->slotNewValue(y);

    if (!isMBPressed)
        getEditWidget()->showGeometry(x, y, -1, -1);
}


void KDlgItem_Base::widgetPaintEvent(QWidget *wid, QPaintEvent *e)
{
    if (role() == Widget) {
        kapp->sendEvent(wid, e);
        if (isSelected()) {
            QPainter p(wid);
            p.setClipRect(e->rect());
            if (wid != widget()) {
                QPoint origin = wid->mapFromParent(QPoint(0,0));
                p.translate((double)origin.x(), (double)origin.y());
            }
            paintCorners(&p);
        }
    } else {
        QPainter p(wid);
        p.setClipRect(e->rect());
        if (wid != widget()) {
            QPoint origin = wid->mapFromParent(QPoint(0,0));
            p.translate((double)origin.x(), (double)origin.y());
        }
        p.eraseRect(wid->rect());
        qDrawPlainRect(&p, wid->rect(), wid->colorGroup().foreground(), 1);
        paintGrid(&p);
        if (isSelected())
            paintCorners(&p);
    }
}


void KDlgItem_Base::widgetMousePressEvent(QWidget *wid, QMouseEvent *e)
{
    QPoint posInWidget(e->pos());
    if (wid != widget())
        posInWidget = wid->mapToParent(e->pos());
    
    getEditWidget()->selectWidget(this);
    
    if (e->button() == LeftButton) {
        getEditWidget()->setModified(true);
        isMBPressed = true;
        startPnt = e->globalPos();
        lastPnt = e->globalPos();
        origRect = widget()->geometry();
        pressedEdge = cornerForPos(posInWidget);
    }
    
    if (e->button() == RightButton) {
            execContextMenu();
    }
}


void KDlgItem_Base::widgetMouseReleaseEvent(QWidget *, QMouseEvent *)
{
    isMBPressed = false;
    KDlgPropWidget *pw = getEditWidget()->getDlgEdit()->kdlg_get_prop_widget();
    if (pw)
        pw->refillList(this);
}


void KDlgItem_Base::widgetMouseMoveEvent(QWidget *wid, QMouseEvent *e)
{
    QPoint posInWidget(e->pos());
    if (wid != widget())
        posInWidget = wid->mapToParent(e->pos());

    moveRulers(posInWidget);
    Corner c = NoCorner;
    if (isSelected())
        if (isMBPressed)
            c = pressedEdge;
        else
            c = cornerForPos(posInWidget);
    setMouseCursorToEdge(c);
    
    if ((!isMBPressed) || (e->globalPos() == lastPnt))
        return;
    int gx = getEditWidget()->gridSizeX();
    int gy = getEditWidget()->gridSizeY();
    int x = origRect.x();
    int y = origRect.y();
    int w = origRect.width();
    int h = origRect.height();
    int diffx = e->globalPos().x() - startPnt.x();
    int diffy = e->globalPos().y() - startPnt.y();
    diffx = ((int)(diffx/gx))*gx;
    diffy = ((int)(diffy/gy))*gy;
    bool noMainWidget = getResizeCoords(pressedEdge, diffx, diffy, &x, &y, &w, &h);
    
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
    if (!parentItem())
        {
            if (x<0) x = 0;
            if (y<0) y = 0;
        }
    if (w<10) w = 10;
    if (h<10) h = 10;
    
    if (isMBPressed)
        getEditWidget()->showGeometry(x, y, w, h);
    
    if (!noMainWidget || role() != Main)
        {
            widget()->setGeometry(x,y,w,h);
            if (role() == Main)
                x = y = 0;
            getProps()->setProp_Value("X",QString().setNum(x));
            getProps()->setProp_Value("Y",QString().setNum(y));
            getProps()->setProp_Value("Width",QString().setNum(w));
            getProps()->setProp_Value("Height",QString().setNum(h));
            if ((role()==Main) && !noMainWidget)
                {
                    getEditWidget()->verticalRuler()->setRange(0,h);
                    getEditWidget()->horizontalRuler()->setRange(0,w);
                }
            AdvListView *lv = getEditWidget()->getDlgEdit()->kdlg_get_prop_widget()->getListView();
            if (lv)
                lv->setGeometryEntries(x,y,w,h);
        }
    lastPnt = e->globalPos();
}


bool KDlgItem_Base::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        // First we send this paint event to the 'real' widget,
        // then we do our own painting. So we must avoid a
        // recursive call of our eventFilter.
        if (inPaintEvent)
            return false;
        inPaintEvent = true;
        widgetPaintEvent((QWidget*)o, (QPaintEvent*)e);
        inPaintEvent = false;
        
        return true;
    } else if (e->type() == QEvent::MouseButtonPress) {
        widgetMousePressEvent((QWidget*)o, (QMouseEvent*)e);
        return true;
    } else if (e->type() == QEvent::MouseButtonRelease) {
        widgetMouseReleaseEvent((QWidget*)o, (QMouseEvent*)e);
        return true;
    } else if (e->type() == QEvent::MouseMove) {
        widgetMouseMoveEvent((QWidget*)o, (QMouseEvent*)e);
        return true;
    }

    return false;
}


void KDlgItem_Base::select()
{
    if (!isSelected()) {
        isItemActive = true;
        widget()->repaint();
        if (widget()->children()) {
            QListIterator<QObject> it(*widget()->children());
            for (; it.current(); ++it)
                if (it.current()->isWidgetType())
                    ((QWidget*)it.current())->repaint();
        }
    }
}


void KDlgItem_Base::deselect()
{
    if (isSelected()) {
        isItemActive = false;
        widget()->repaint();
        if (widget()->children()) {
            QListIterator<QObject> it(*widget()->children());
            for (; it.current(); ++it)
                if (it.current()->isWidgetType())
                    ((QWidget*)it.current())->repaint();
        }
    }
}
