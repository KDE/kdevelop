#ifdef CALLER_ITEMCLASS_NAME

#include "../ckdevelop.h"
#include "kdlgproplv.h"


int CALLER_ITEMCLASS_NAME::MyWidget::recPosX(int px)
{
    int xnew = px+x(); 
    if ((!parent()) || (parent()!=parentObject->getEditWidget()))
	return ((KDlgItem_Widget::MyWidget*)parent())->recPosX(xnew);
    else
	return px;
}


int CALLER_ITEMCLASS_NAME::MyWidget::recPosY(int py)
{
    int ynew = py+y();
    if ((!parent()) || (parent()!=parentObject->getEditWidget()))
	return py;
}


void CALLER_ITEMCLASS_NAME::MyWidget::deselect()
{
    if (isItemActive)
	{
	    isItemActive = false;
	    repaint();
	}
}


void CALLER_ITEMCLASS_NAME::MyWidget::select()
{
    if (!isItemActive)
	{
	    isItemActive = true;
	    repaint();
	}
}


void CALLER_ITEMCLASS_NAME::MyWidget::selectMe()
{
    parentObject->getEditWidget()->selectWidget((KDlgItem_Base*)parentObject);
}


void CALLER_ITEMCLASS_NAME::MyWidget::mousePressEvent ( QMouseEvent *e )
{
  selectMe();

  if (e->button() == LeftButton)
    {
      parentObject->getEditWidget()->setModified(true);
      isMBPressed = true;
      startPnt = e->globalPos();
      lastPnt = e->globalPos();
      origRect = geometry();
      pressedEdge = KDlgItemsGetClickedRect(e->pos().x(), e->pos().y(), ((QWidget*)this)->width(), ((QWidget*)this)->height());
    }

  if (e->button() == RightButton)
    {
      parentObject->execContextMenu(isMainwidget);
    }

}

void CALLER_ITEMCLASS_NAME::MyWidget::moveRulers( QMouseEvent *e )
{
  if (!parentObject)
    return;

  int gx = parentObject->getEditWidget()->gridSizeX();
  int gy = parentObject->getEditWidget()->gridSizeY();
  int x = e->pos().x()+recPosX(0);
  int y = e->pos().y()+recPosY(0);
  x = ((int)(x/gx))*gx;
  y = ((int)(y/gy))*gy;
  parentObject->getEditWidget()->horizontalRuler()->slotNewValue(x);
  parentObject->getEditWidget()->verticalRuler()->slotNewValue(y);

  if (!isMBPressed)
    {
      QString xy = QString().sprintf(i18n("X:%.3d Y:%.3d "), x, y);
      parentObject->getEditWidget()->getCKDevel()->kdlg_get_statusbar()->changeItem((const char*)xy, ID_KDLG_STATUS_XY);
      parentObject->getEditWidget()->getCKDevel()->kdlg_get_statusbar()->changeItem("", ID_KDLG_STATUS_WH);
    }
}

void CALLER_ITEMCLASS_NAME::MyWidget::mouseReleaseEvent ( QMouseEvent * )
{
  isMBPressed = false;
  KDlgPropWidget *pw = parentObject->getEditWidget()->getCKDevel()->kdlg_get_prop_widget();
  if (pw)
    pw->refillList(parentObject);
}


void CALLER_ITEMCLASS_NAME::MyWidget::mouseMoveEvent ( QMouseEvent *e )
{
  moveRulers(e);
  if (isItemActive)
    {
       int pE;
       if (isMBPressed) pE = pressedEdge; else pE = KDlgItemsGetClickedRect(e->pos().x(), e->pos().y(), ((QWidget*)this)->width(), ((QWidget*)this)->height());
       KDlgItemsSetMouseCursor(this, pE);
    }
  else
    setCursor(KCursor::arrowCursor());

  if ((!isMBPressed) || (e->pos() == lastPnt)) return;
  int gx = parentObject->getEditWidget()->gridSizeX();
  int gy = parentObject->getEditWidget()->gridSizeY();
  int x = origRect.x();
  int y = origRect.y();
  int w = origRect.width();
  int h = origRect.height();
  int diffx = e->globalPos().x() - startPnt.x();
  int diffy = e->globalPos().y() - startPnt.y();
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
  if (!parentObject->parentWidgetItem)
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
      parentObject->getEditWidget()->getCKDevel()->kdlg_get_statusbar()->changeItem((const char*)xy, ID_KDLG_STATUS_XY);
      parentObject->getEditWidget()->getCKDevel()->kdlg_get_statusbar()->changeItem((const char*)wh, ID_KDLG_STATUS_WH);
    }

  if ((!noMainWidget) || (!isMainwidget))
    {
      setGeometry(x,y,w,h);
      if (isMainwidget)
        {
          x = 0;
          y = 0;
        }
      parentObject->getProps()->setProp_Value("X",QString().setNum(x));
      parentObject->getProps()->setProp_Value("Y",QString().setNum(y));
      parentObject->getProps()->setProp_Value("Width",QString().setNum(w));
      parentObject->getProps()->setProp_Value("Height",QString().setNum(h));
      if (isMainwidget && !noMainWidget) { parentObject->getEditWidget()->verticalRuler()->setRange(0,h); parentObject->getEditWidget()->horizontalRuler()->setRange(0,w);}
      AdvListView *lv = parentObject->getEditWidget()->getCKDevel()->kdlg_get_prop_widget()->getListView();
      if (lv)
        lv->setGeometryEntrys(x,y,w,h);
  }
  lastPnt = e->pos();
}

#undef CALLER_ITEMCLASS_NAME
#endif
