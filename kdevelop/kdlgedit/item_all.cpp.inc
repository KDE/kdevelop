#ifdef CALLER_ITEMCLASS_NAME

#include "../ckdevelop.h"
#include "kdlgproplv.h"

void CALLER_ITEMCLASS_NAME::MyWidget::mousePressEvent ( QMouseEvent *e )
{
  selectMe();
  isMBPressed = true;
  startPnt = e->globalPos();
  lastPnt = e->globalPos();
  origRect = geometry();
  pressedEdge = KDlgItemsGetClickedRect(e->pos().x(), e->pos().y(), width(), height());
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
       if (isMBPressed) pE = pressedEdge; else pE = KDlgItemsGetClickedRect(e->pos().x(), e->pos().y(), width(), height());
       KDlgItemsSetMouseCursor(this, pE);
    }
  else setCursor(KCursor::arrowCursor());
  if ((!isMBPressed) || (e->pos() == lastPnt)) return;
  int x = origRect.x();
  int y = origRect.y();
  int w = origRect.width();
  int h = origRect.height();
  int diffx = e->globalPos().x() - startPnt.x();
  int diffy = e->globalPos().y() - startPnt.y();
  bool noMainWidget;
  noMainWidget = KDlgItemsGetResizeCoords(pressedEdge, x, y, w, h, diffx, diffy);
  if (w<0) w=0;
  if (h<0) h=0;
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