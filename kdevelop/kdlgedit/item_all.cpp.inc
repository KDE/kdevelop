#ifdef CALLER_ITEMCLASS_NAME

#include "../ckdevelop.h"
#include "kdlgproplv.h"

void CALLER_ITEMCLASS_NAME::MyWidget::mousePressEvent ( QMouseEvent *e )
{
  selectMe();

  if (e->button() == LeftButton)
    {
      isMBPressed = true;
      startPnt = e->globalPos();
      lastPnt = e->globalPos();
      origRect = geometry();
      pressedEdge = KDlgItemsGetClickedRect(e->pos().x(), e->pos().y(), width(), height());
    }

  if (e->button() == RightButton)
    {
      #define mkQPixTb(fn) QPixmap(KApplication::kde_toolbardir() + QString("/") +fn)
      #define mkQPixDd(fn) QPixmap(KApplication::kde_datadir() + QString("/kdevelop/toolbar/") + fn)


      KPopupMenu phelp;
      phelp.setTitle( parentObject->itemClass() );
      if (!isMainwidget)
        {
          phelp.insertItem( mkQPixTb("prev.xpm"), i18n("&Raise"), parentObject->getEditWidget(), SLOT(slot_raiseSelected()) );
          phelp.insertItem( mkQPixTb("next.xpm"), i18n("&Lower"), parentObject->getEditWidget(), SLOT(slot_lowerSelected()) );
          phelp.insertItem( mkQPixTb("top.xpm"), i18n("Raise to &top"),    parentObject->getEditWidget(), SLOT(slot_raiseTopSelected()) );
          phelp.insertItem( mkQPixTb("bottom.xpm"), i18n("Lower to &bottom"), parentObject->getEditWidget(), SLOT(slot_lowerBottomSelected()) );
          phelp.insertSeparator();
          phelp.insertItem( mkQPixDd("cut.xpm"), i18n("C&ut"),   parentObject->getEditWidget(), SLOT(slot_cutSelected()) );
          phelp.insertItem( mkQPixTb("delete.xpm"), i18n("&Delete"),parentObject->getEditWidget(), SLOT(slot_deleteSelected()) );
          phelp.insertItem( mkQPixDd("copy.xpm"), i18n("&Copy"),  parentObject->getEditWidget(), SLOT(slot_copySelected()) );
        }
      phelp.insertItem( mkQPixDd("paste.xpm"), i18n("&Paste"), parentObject->getEditWidget(), SLOT(slot_pasteSelected()) );
      phelp.insertSeparator();
      phelp.insertItem( mkQPixTb("help.xpm"), i18n("&Help"),  parentObject->getEditWidget(), SLOT(slot_helpSelected()) );
      phelp.exec(QCursor::pos());
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
  if (w<0) w=0;
  if (h<0) h=0;
  if ((!noMainWidget) || (!isMainwidget))
    {
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