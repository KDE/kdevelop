/***************************************************************************
                             items.cpp 
                             -------------------                                         
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "items.h"
#include "itemsglobal.h"


/***************************************************************************
 *  QWidget                                                                *
 ***************************************************************************/


KDlgItem_QWidget::MyWidget::MyWidget(KDlgItem_QWidget *wid, QWidget *parent,
                                     bool isMainWidget, const char *name)
  : QFrame(parent, name)
{
  parentObject = wid;
  if (isMainWidget)
    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
  else
    {
      setFrameStyle( QFrame::Panel | QFrame::Plain );
      setLineWidth(1);
    }
  show();
}

#include <iostream.h>
void KDlgItem_QWidget::MyWidget::paintEvent( QPaintEvent *e)
{
    cout << "paint on container widget " << endl;
  QFrame::paintEvent(e);

  QPainter p(this);
  p.setClipRect(e->rect());

  int x,y;
  int gx = parentObject->getEditWidget()->gridSizeX();
  int gy = parentObject->getEditWidget()->gridSizeY();

  if ((gx<=1) || (gy<=1))
    {
      if ((gx>1) || (gy>1))
        {
          QPen oldpen = p.pen();
          QPen newpen(QColor(255,128,128),0,DashDotLine);
          p.setPen(newpen);
          if (gx <= 1)
            {
              for (y = 0; y < height(); y+=gy)
                p.drawLine(3,y,width()-6,y);
            }
          else
            {
              for (x = 0; x < width(); x+=gx)
                p.drawLine(x,3,x,height()-6);
            }
          p.setPen(oldpen);
        }
    }
  else
    {
      for (x = 0; x < width(); x+=gx)
       for (y = 0; y < height(); y+=gy)
         p.drawPoint(x,y);
    }


  if (parentObject->isItemActive)
      parentObject->paintCorners(&p);
  //    KDlgItemsPaintRects(&p,width(),height());
}


KDlgItem_QWidget::KDlgItem_QWidget( KDlgEditWidget *editwid, QWidget *parent,
                                    bool ismainwidget, const char *name)
   : KDlgItem_Base(editwid, parent, ismainwidget, name)
{
  parentWidgetItem = 0;
  childs = new KDlgItemDatabase();
  item = new MyWidget(this, parent, isMainwidget);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
}

KDlgItem_QWidget::KDlgItem_QWidget(KDlgEditWidget *editwid, QWidget *parent, const char *name)
   : KDlgItem_Base(editwid, parent, false, name)
{
  parentWidgetItem = 0;
  childs = new KDlgItemDatabase();
  item = new MyWidget(this, parent, false);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
}

void KDlgItem_QWidget::addMyPropEntrys()
{
}

void KDlgItem_QWidget::recreateItem()
{
  KDlgItem_Base *it = childs->getFirst();
  while (it)
  {
    ((KDlgItem_QWidget*)it)->recreateItem();
    it = childs->getNext();
  }

  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
  return;
}

void KDlgItem_QWidget::repaintItem(QFrame *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  if (isMainwidget)
    {
      getEditWidget()->horizontalRuler()->setRange(0,item->width());
      getEditWidget()->verticalRuler()->setRange(0,item->height());
    }


}


/***************************************************************************
 *  QLineEdit                                                              *
 ***************************************************************************/


KDlgItem_QLineEdit::KDlgItem_QLineEdit(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QLineEdit(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QLineEdit::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QLineEdit::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Text",           "",       "General",        ALLOWED_STRING);
  props->addProp("hasFrame",       "",       "General",        ALLOWED_BOOL);
  props->addProp("MaxLength",      "",       "General",        ALLOWED_UINT);
  props->addProp("CursorPosition", "",       "General",        ALLOWED_INT);
  props->addProp("isTextSelected", "",       "General",        ALLOWED_BOOL);
}

void KDlgItem_QLineEdit::repaintItem(QLineEdit *it)
{
  QLineEdit *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  if (!Prop2Str("Text").isNull())
    itm->setText(Prop2Str("Text"));

  if (!Prop2Str("MaxLength").isEmpty())
      //    if(Prop2Int("MaxLength") >=0){
      itm->setMaxLength(Prop2Int("MaxLength",32767));
  //    }

  else
    itm->setMaxLength(32767);

  if (!Prop2Str("CursorPosition").isEmpty())
    itm->setCursorPosition(Prop2Int("CursorPosition",32767));
  else
    itm->setCursorPosition(0);

  itm->setFrame(Prop2Bool("hasFrame") == 1 ? TRUE : FALSE);

  if (Prop2Bool("isTextSelected") == 1)
    itm->selectAll();
  else
    itm->deselect();

}


/***************************************************************************
 *  QPushButton                                                            *
 ***************************************************************************/


KDlgItem_QPushButton::KDlgItem_QPushButton(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QPushButton(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();

  isMainwidget = false;
}

void KDlgItem_QPushButton::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QPushButton::addMyPropEntrys()
{

  props->addProp("Text",           "Button",       "General",        ALLOWED_STRING);
  props->addProp("isDefault",      "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoDefault" , "",             "General",        ALLOWED_BOOL);
  props->addProp("isToggleButton", "",             "General",        ALLOWED_BOOL);
  props->addProp("isToggledOn",    "",             "General",        ALLOWED_BOOL);
  props->addProp("isMenuButton",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoResize",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoRepeat",   "",             "General",        ALLOWED_BOOL);

  props->addProp("Pixmap",         "",             "Appearance",     ALLOWED_FILE);
}

void KDlgItem_QPushButton::repaintItem(QPushButton *it)
{
  QPushButton *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  if (!Prop2Str("Text").isNull())
    itm->setText(Prop2Str("Text"));

  if (Prop2Str("Pixmap").isEmpty())
    {
      if (itm->pixmap())
        itm->setPixmap(QPixmap());
    }
  else
    itm->setPixmap(QPixmap(Prop2Str("Pixmap")));

  itm->setDefault(Prop2Bool("isDefault") == 1 ? TRUE : FALSE);
  itm->setIsMenuButton(Prop2Bool("isMenuButton") == 1 ? TRUE : FALSE);
  itm->setAutoResize(Prop2Bool("isAutoResize") == 1 ? TRUE : FALSE);
  itm->setToggleButton(Prop2Bool("isToggleButton") == 1 ? TRUE : FALSE);
  itm->setOn((Prop2Bool("isToggledOn") == 1 ? TRUE : FALSE) && (Prop2Bool("isToggleButton") == 1 ? TRUE : FALSE));
}


/***************************************************************************
 *  QLabel                                                                 *
 ***************************************************************************/


KDlgItem_QLabel::KDlgItem_QLabel(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QLabel(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QLabel::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QLabel::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Text",           "Label",       "General",        ALLOWED_STRING);
  props->addProp("Buddy",          ""     ,       "C++ Code",       ALLOWED_STRING);
  props->addProp("isAutoResize",   "",            "General",        ALLOWED_BOOL);
  props->addProp("Margin",         "",            "Appearance",     ALLOWED_INT);
}

void KDlgItem_QLabel::repaintItem(QLabel *it)
{
  QLabel *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  if (!Prop2Str("Text").isNull())
    itm->setText(Prop2Str("Text"));

  itm->setAutoResize(Prop2Bool("isAutoResize") == 1 ? TRUE : FALSE);

  itm->setMargin(props->getIntFromProp("Margin",-1));

}


/***************************************************************************
 *  QLCDNumber                                                             *
 ***************************************************************************/


KDlgItem_QLCDNumber::KDlgItem_QLCDNumber(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QLCDNumber(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QLCDNumber::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QLCDNumber::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Value",           "0",       "General",        ALLOWED_INT);
  props->addProp("NumDigits",       "",        "General",        ALLOWED_INT);
}

void KDlgItem_QLCDNumber::repaintItem(QLCDNumber *it)
{
  QLCDNumber *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  int i = props->getIntFromProp("NumDigits",4);
  if (i<=0) i=1;
  itm->setNumDigits(i);

  itm->display(props->getIntFromProp("Value",0));
}


/***************************************************************************
 *  QRadioButton                                                           *
 ***************************************************************************/


KDlgItem_QRadioButton::KDlgItem_QRadioButton(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QRadioButton(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QRadioButton::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QRadioButton::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Text",           "RadioButton",       "General",        ALLOWED_STRING);
  props->addProp("isChecked",      "",                  "General",        ALLOWED_BOOL);
  props->addProp("isAutoResize",   "",                  "General",        ALLOWED_BOOL);
  props->addProp("isAutoRepeat",   "",                  "General",        ALLOWED_BOOL);

  props->addProp("Pixmap",         "",             "Appearance",     ALLOWED_FILE);
}

void KDlgItem_QRadioButton::repaintItem(QRadioButton *it)
{
  QRadioButton *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  if (!Prop2Str("Text").isNull())
    itm->setText(Prop2Str("Text"));

  if (Prop2Str("Pixmap").isEmpty())
    {
      if (itm->pixmap())
        itm->setPixmap(QPixmap());
    }
  else
    itm->setPixmap(QPixmap(Prop2Str("Pixmap")));

  itm->setChecked(Prop2Bool("isChecked") == 1 ? TRUE : FALSE);
  itm->setAutoResize(Prop2Bool("isAutoResize") == 1 ? TRUE : FALSE);
}


/***************************************************************************
 *  QCheckBox                                                              *
 ***************************************************************************/


KDlgItem_QCheckBox::KDlgItem_QCheckBox(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QCheckBox(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QCheckBox::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QCheckBox::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Text",           "CheckBox",       "General",        ALLOWED_STRING);
  props->addProp("isChecked",      "",               "General",        ALLOWED_BOOL);
  props->addProp("isAutoResize",   "",               "General",        ALLOWED_BOOL);
  props->addProp("isAutoRepeat",   "",               "General",        ALLOWED_BOOL);

  props->addProp("Pixmap",         "",             "Appearance",     ALLOWED_FILE);
}

void KDlgItem_QCheckBox::repaintItem(QCheckBox *it)
{
  QCheckBox *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  if (!Prop2Str("Text").isNull())
    itm->setText(Prop2Str("Text"));

  if (Prop2Str("Pixmap").isEmpty())
    {
      if (itm->pixmap())
        itm->setPixmap(QPixmap());
    }
  else
    itm->setPixmap(QPixmap(Prop2Str("Pixmap")));

  itm->setChecked(Prop2Bool("isChecked") == 1 ? TRUE : FALSE);
  itm->setAutoResize(Prop2Bool("isAutoResize") == 1 ? TRUE : FALSE);
}


/***************************************************************************
 *  QComboBox                                                              *
 ***************************************************************************/


KDlgItem_QComboBox::KDlgItem_QComboBox(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QComboBox(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QComboBox::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QComboBox::addMyPropEntrys()
{
  if (!props)
    return;
  props->addProp("isAutoResize",   "",                  "General",        ALLOWED_BOOL);
  props->addProp("Entries",        "",                  "General",        ALLOWED_MULTISTRING);
  props->addProp("CurrentItem",    "",                  "General",        ALLOWED_INT);
  props->addProp("SizeLimit",      "",                  "General",        ALLOWED_INT);
  props->addProp("MaxCount",       "",                  "General",        ALLOWED_INT);
  props->addProp("InsertionPolicy","AtTop",             "General",        ALLOWED_COMBOLIST,
                            "NoInsertion\nAtTop\nAtCurrent\nAtBottom\nAfterCurrent\nBeforeCurrent");
  props->addProp("isAutoCompletion",  "",               "General",        ALLOWED_BOOL);
  props->addProp("EditText",       "",                  "General",        ALLOWED_STRING);

}

void KDlgItem_QComboBox::repaintItem(QComboBox *it)
{
  QComboBox *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
  itm->setAutoResize(Prop2Bool("isAutoResize") == 1 ? TRUE : FALSE);

  itm->clear();

  int i = 0;
  QString src = Prop2Str("Entries");
  if(src != ""){
    
    QString s;
    s = getLineOutOfString(src,i,"\\n");
    while (!s.isNull()){
	itm->insertItem(s);
	i++;
	s = getLineOutOfString(src,i,"\\n");
      }
  }
}


/***************************************************************************
 *  QListBox                                                               *
 ***************************************************************************/


KDlgItem_QListBox::KDlgItem_QListBox(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QListBox(parent, name);
  item->installEventFilter(this);
  item->setMouseTracking(true);
  ((QScrollView*)item)->viewport()->installEventFilter(this);
  ((QScrollView*)item)->viewport()->setMouseTracking(true);
  item->show();
  ((QScrollView*)item)->viewport()->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QListBox::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
  ((QScrollView*)item)->viewport()->setMouseTracking(true);
}

void KDlgItem_QListBox::addMyPropEntrys()
{
  if (!props)
    return;
  props->addProp("isAutoUpdate",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoScroll",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoScrollBar",   "",          "General",        ALLOWED_BOOL);
  props->addProp("isAutoBottomScrollBar",   "",          "General",        ALLOWED_BOOL);
  props->addProp("isBottomScrollBar",   "",          "General",        ALLOWED_BOOL);
  props->addProp("isDragSelect",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isSmoothScrolling",   "",             "General",        ALLOWED_BOOL);
  props->addProp("Entries",            "",              "General",        ALLOWED_MULTISTRING);

  props->addProp("setFixedVisibleLines",   "",             "Geometry",        ALLOWED_INT);
}

void KDlgItem_QListBox::repaintItem(QListBox *it)
{
  QListBox *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
  itm->setAutoUpdate(Prop2Bool("isAutoUpdate") == 1 ? TRUE : FALSE);
  itm->setAutoScroll(Prop2Bool("isAutoScroll") == 1 ? TRUE : FALSE);
  itm->setAutoScrollBar(Prop2Bool("isAutoScrollBar") == 1 ? TRUE : FALSE);
  itm->setAutoBottomScrollBar(Prop2Bool("isAutoBottomScrollBar") == 1 ? TRUE : FALSE);
  itm->setBottomScrollBar(Prop2Bool("isBottomScrollBar") == 1 ? TRUE : FALSE);
  itm->setDragSelect(Prop2Bool("isDragSelect") == 1 ? TRUE : FALSE);
  itm->setSmoothScrolling(Prop2Bool("isSmoothScrolling") == 1 ? TRUE : FALSE);
  if(Prop2Str("setFixedVisibleLines") != ""){
    itm->setFixedVisibleLines(Prop2Int("setFixedVisibleLines"));
  }
}


/***************************************************************************
 *  QMultiLineEdit                                                         *
 ***************************************************************************/


KDlgItem_QMultiLineEdit::KDlgItem_QMultiLineEdit(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QMultiLineEdit(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QMultiLineEdit::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QMultiLineEdit::addMyPropEntrys()
{
  if (!props)
    return;
  props->addProp("isAutoUpdate",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isReadOnly",   "",                  "General",        ALLOWED_BOOL);
  props->addProp("isOverWriteMode",   "",                  "General",        ALLOWED_BOOL);
  props->addProp("Text",           "",       "General",        ALLOWED_MULTISTRING);
  props->addProp("isTextSelected", "",       "General",        ALLOWED_BOOL);
  props->addProp("setFixedVisibleLines",   "",             "Geometry",        ALLOWED_INT);
}

void KDlgItem_QMultiLineEdit::repaintItem(QMultiLineEdit *it)
{
  QMultiLineEdit *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  itm->setAutoUpdate(Prop2Bool("isAutoUpdate") == 1 ? TRUE : FALSE);
  itm->setReadOnly(Prop2Bool("isReadOnly") == 1 ? TRUE : FALSE);
  itm->setOverwriteMode(Prop2Bool("isOverWriteMode") == 1 ? TRUE : FALSE);

  if (!Prop2Str("Text").isNull())
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


/***************************************************************************
 *  QProgressBar                                                           *
 ***************************************************************************/


KDlgItem_QProgressBar::KDlgItem_QProgressBar(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QProgressBar(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QProgressBar::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QProgressBar::addMyPropEntrys()
{
  if (!props)
    return;
  
  props->addProp("TotalSteps",      "",        "General",        ALLOWED_INT);
  
}

void KDlgItem_QProgressBar::repaintItem(QProgressBar *it)
{
  QProgressBar *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
}


/***************************************************************************
 *  QSpinBox                                                               *
 ***************************************************************************/


KDlgItem_QSpinBox::KDlgItem_QSpinBox(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  //  item = new MyWidget(this, parent);
  item = new QSpinBox(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QSpinBox::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QSpinBox::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Prefix",               "",            "General",        ALLOWED_STRING);
  props->addProp("Suffix",               "",            "General",        ALLOWED_STRING);
  props->addProp("Value",                "",            "General",        ALLOWED_INT);
  props->addProp("MinValue",             "0",           "General",        ALLOWED_INT);
  props->addProp("MaxValue",             "99",          "General",        ALLOWED_INT);
  props->addProp("isWrapping",           "",            "General",        ALLOWED_BOOL);
  props->addProp("SpecialValText",       ""     ,       "General",        ALLOWED_STRING);
  
}

void KDlgItem_QSpinBox::repaintItem(QSpinBox *it)
{
  QSpinBox *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  if (!Prop2Str("Suffix").isNull())
    itm->setSuffix(Prop2Str("Suffix"));
  if (!Prop2Str("Prefix").isNull())
    itm->setPrefix(Prop2Str("Prefix"));

  itm->setWrapping(Prop2Bool("isWrapping") == 1 ? TRUE : FALSE);

  if(!Prop2Str("Value").isEmpty())
    itm->setValue(Prop2Int("Value"));
  
  itm->setRange(Prop2Int("MinValue"),Prop2Int("MaxValue"));
}


/***************************************************************************
 *  QSlider                                                                *
 ***************************************************************************/


KDlgItem_QSlider::KDlgItem_QSlider(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QSlider(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QSlider::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QSlider::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Value",      "",        "General",        ALLOWED_INT);
  props->addProp("MinValue",      "0",        "General",        ALLOWED_INT);
  props->addProp("MaxValue",      "99",        "General",        ALLOWED_INT);
  props->addProp("isTracking",           "",       "General",        ALLOWED_BOOL);
  props->addProp("Orientation",           "Horizontal",       "General",        ALLOWED_ORIENTATION);
  
}

void KDlgItem_QSlider::repaintItem(QSlider *it)
{
  QSlider *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  itm->setTracking(Prop2Bool("isTracking") == 1 ? TRUE : FALSE);
  
  if(!Prop2Str("Value").isEmpty())
    itm->setValue(Prop2Int("Value"));
  
  itm->setRange(Prop2Int("MinValue"),Prop2Int("MaxValue"));
  if(Prop2Str("Orientation") == "Horizontal"){
    itm->setOrientation(QSlider::Horizontal);
  }
  else{
    itm->setOrientation(QSlider::Vertical);
  }
}


/***************************************************************************
 *  QScrollBar                                                             *
 ***************************************************************************/


KDlgItem_QScrollBar::KDlgItem_QScrollBar(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QScrollBar(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QScrollBar::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QScrollBar::addMyPropEntrys()
{
  if (!props)
    return;
  props->addProp("Value",      "",        "General",        ALLOWED_INT);
  props->addProp("MinValue",      "0",        "General",        ALLOWED_INT);
  props->addProp("MaxValue",      "99",        "General",        ALLOWED_INT);
  props->addProp("isTracking",           "",       "General",        ALLOWED_BOOL);
  props->addProp("Orientation",           "Horizontal",       "General",        ALLOWED_ORIENTATION);
  
}

void KDlgItem_QScrollBar::repaintItem(QScrollBar *it)
{
  QScrollBar *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  itm->setTracking(Prop2Bool("isTracking") == 1 ? TRUE : FALSE);
  
  if (!Prop2Str("Value").isEmpty())
    itm->setValue(Prop2Int("Value"));
  
  itm->setRange(Prop2Int("MinValue"),Prop2Int("MaxValue"));

  if(Prop2Str("Orientation") == "Horizontal"){
    itm->setOrientation(QScrollBar::Horizontal);
  }
  else{
    itm->setOrientation(QScrollBar::Vertical);
  }
}


/***************************************************************************
 *  QGroupBox                                                              *
 ***************************************************************************/


KDlgItem_QGroupBox::KDlgItem_QGroupBox(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QGroupBox(parent, name);
  item->installEventFilter(this);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QGroupBox::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_QGroupBox::addMyPropEntrys()
{
  if (!props)
    return;
  props->addProp("Title",           "",       "General",        ALLOWED_STRING);
  
}

void KDlgItem_QGroupBox::repaintItem(QGroupBox *it)
{
  QGroupBox *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  if (!Prop2Str("Title").isNull())
    itm->setTitle(Prop2Str("Title"));
}


/***************************************************************************
 *  QListView                                                              *
 ***************************************************************************/


KDlgItem_QListView::KDlgItem_QListView(KDlgEditWidget *editwid, QWidget *parent, const char *name)
  : KDlgItem_Base(editwid, parent, false, name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new QListView(parent, name);
  item->installEventFilter(this);
  item->setMouseTracking(true);
  ((QScrollView*)item)->viewport()->installEventFilter(this);
  ((QScrollView*)item)->viewport()->setMouseTracking(true);
  item->show();
  repaintItem();
  isMainwidget = false;
}

void KDlgItem_QListView::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
  ((QScrollView*)item)->viewport()->setMouseTracking(true);
}

void KDlgItem_QListView::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Entries",            "",              "General",        ALLOWED_MULTISTRING);

  props->addProp("vScrollBarMode",     "Auto",          "Appearance",     ALLOWED_COMBOLIST, "Auto\nAlwaysOff\nAlwaysOn");
  props->addProp("hScrollBarMode",     "Auto",          "Appearance",     ALLOWED_COMBOLIST, "Auto\nAlwaysOff\nAlwaysOn");
  props->addProp("TreeStepSize",       "",              "Appearance",     ALLOWED_INT);
  props->addProp("ItemMargin",         "",              "Appearance",     ALLOWED_INT);
  props->addProp("Columns",            "",              "Appearance",     ALLOWED_MULTISTRING);
  props->addProp("isMultiSelection",   "",              "Appearance",     ALLOWED_BOOL);
  props->addProp("isAllColumnsShowFocus", "",           "Appearance",     ALLOWED_BOOL);
  props->addProp("isRootDecorated",    "",              "Appearance",     ALLOWED_BOOL);
}

void KDlgItem_QListView::repaintItem(QListView *it)
{
  QListView *itm = it ? it : getItem();

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
}
