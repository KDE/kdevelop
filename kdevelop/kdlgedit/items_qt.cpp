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


KDlgItem_QWidget::KDlgItem_QWidget(KDlgEditWidget *editwid, Role role, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, role)
{
    QWidget *parent = parentit? parentit->widget() : editwid;
    setWidget(new QWidget(parent, "KDlgEdit Main"));
    childs = new KDlgItemDatabase();
}


KDlgItem_QWidget::KDlgItem_QWidget(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
   : KDlgItem_Base(editwid, parentit, Container)
{
    QWidget *parent = parentit? parentit->widget() : editwid;
    setWidget(new QWidget(parent, "KDlgEdit Container"));
    childs = new KDlgItemDatabase();
}


void KDlgItem_QWidget::addMyPropEntrys()
{
}


void KDlgItem_QWidget::repaintItem()
{
    KDlgItem_Base::repaintItem();

    if (role() == Main) {
        getEditWidget()->horizontalRuler()->setRange(0,widget()->width());
        getEditWidget()->verticalRuler()->setRange(0,widget()->height());
    }
}


/***************************************************************************
 *  QLineEdit                                                              *
 ***************************************************************************/


KDlgItem_QLineEdit::KDlgItem_QLineEdit(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QLineEdit(parentit->widget(), "KDlgEdit QLineEdit"));
    addMyPropEntrys();
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

void KDlgItem_QLineEdit::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QLineEdit *itm = (QLineEdit*)widget();

  if (!Prop2Str("Text").isNull())
    itm->setText(Prop2Str("Text"));

  if (!Prop2Str("MaxLength").isEmpty())
      itm->setMaxLength(Prop2Int("MaxLength",32767));

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


KDlgItem_QPushButton::KDlgItem_QPushButton(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QPushButton(parentit->widget(), "KDlgEdit QPushButton"));
    addMyPropEntrys();
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

void KDlgItem_QPushButton::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QPushButton *itm = (QPushButton*)widget();

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


KDlgItem_QLabel::KDlgItem_QLabel(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QLabel(parentit->widget(), "KDlgEdit QLineEdit"));
    addMyPropEntrys();
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

void KDlgItem_QLabel::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QLabel *itm = (QLabel*)widget();

  if (!Prop2Str("Text").isNull())
    itm->setText(Prop2Str("Text"));

  itm->setAutoResize(Prop2Bool("isAutoResize") == 1 ? TRUE : FALSE);

  itm->setMargin(props->getIntFromProp("Margin",-1));

}


/***************************************************************************
 *  QLCDNumber                                                             *
 ***************************************************************************/


KDlgItem_QLCDNumber::KDlgItem_QLCDNumber(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QLCDNumber(parentit->widget(), "KDlgEdit QLineEdit"));
    addMyPropEntrys();
}

void KDlgItem_QLCDNumber::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Value",           "0",       "General",        ALLOWED_INT);
  props->addProp("NumDigits",       "",        "General",        ALLOWED_INT);
}

void KDlgItem_QLCDNumber::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QLCDNumber *itm = (QLCDNumber*)widget();

  int i = props->getIntFromProp("NumDigits",4);
  if (i<=0) i=1;
  itm->setNumDigits(i);

  itm->display(props->getIntFromProp("Value",0));
}


/***************************************************************************
 *  QRadioButton                                                           *
 ***************************************************************************/


KDlgItem_QRadioButton::KDlgItem_QRadioButton(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QLineEdit(parentit->widget(), "KDlgEdit QRadioButton"));
    addMyPropEntrys();
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

void KDlgItem_QRadioButton::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QRadioButton *itm = (QRadioButton*)widget();

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


KDlgItem_QCheckBox::KDlgItem_QCheckBox(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QCheckBox(parentit->widget(), "KDlgEdit QLineEdit"));
    addMyPropEntrys();
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

void KDlgItem_QCheckBox::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QCheckBox *itm = (QCheckBox*)widget();

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


KDlgItem_QComboBox::KDlgItem_QComboBox(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QComboBox(parentit->widget(), "KDlgEdit QComboBox"));
    addMyPropEntrys();
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

void KDlgItem_QComboBox::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QComboBox *itm = (QComboBox*)widget();

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


KDlgItem_QListBox::KDlgItem_QListBox(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QListBox(parentit->widget(), "KDlgEdit QListBox"));
    addMyPropEntrys();
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

void KDlgItem_QListBox::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QListBox *itm = (QListBox*)widget();

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


KDlgItem_QMultiLineEdit::KDlgItem_QMultiLineEdit(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QMultiLineEdit(parentit->widget(), "KDlgEdit QMultiLineEdit"));
    addMyPropEntrys();
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

void KDlgItem_QMultiLineEdit::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QMultiLineEdit *itm = (QMultiLineEdit*)widget();

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


KDlgItem_QProgressBar::KDlgItem_QProgressBar(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QProgressBar(parentit->widget(), "KDlgEdit QLineEdit"));
    addMyPropEntrys();
}

void KDlgItem_QProgressBar::addMyPropEntrys()
{
  if (!props)
    return;
  
  props->addProp("TotalSteps",      "",        "General",        ALLOWED_INT);
  
}

void KDlgItem_QProgressBar::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QProgressBar *itm = (QProgressBar*)widget();
}


/***************************************************************************
 *  QSpinBox                                                               *
 ***************************************************************************/


KDlgItem_QSpinBox::KDlgItem_QSpinBox(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QSpinBox(parentit->widget(), "KDlgEdit QSpinBox"));
    addMyPropEntrys();
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

void KDlgItem_QSpinBox::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QSpinBox *itm = (QSpinBox*)widget();


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


KDlgItem_QSlider::KDlgItem_QSlider(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QSlider(parentit->widget(), "KDlgEdit QSlider"));
    addMyPropEntrys();
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

void KDlgItem_QSlider::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QSlider *itm = (QSlider*)widget();

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


KDlgItem_QScrollBar::KDlgItem_QScrollBar(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QScrollBar(parentit->widget(), "KDlgEdit QScrollBar"));
    addMyPropEntrys();
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

void KDlgItem_QScrollBar::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QScrollBar *itm = (QScrollBar*)widget();

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


KDlgItem_QGroupBox::KDlgItem_QGroupBox(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QGroupBox(parentit->widget(), "KDlgEdit QGroupBox"));
    addMyPropEntrys();
}

void KDlgItem_QGroupBox::addMyPropEntrys()
{
  if (!props)
    return;
  props->addProp("Title",           "",       "General",        ALLOWED_STRING);
  
}

void KDlgItem_QGroupBox::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QGroupBox *itm = (QGroupBox*)widget();

  if (!Prop2Str("Title").isNull())
    itm->setTitle(Prop2Str("Title"));
}


/***************************************************************************
 *  QListView                                                              *
 ***************************************************************************/


KDlgItem_QListView::KDlgItem_QListView(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new QListView(parentit->widget(), "KDlgEdit QListView"));
    addMyPropEntrys();
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

void KDlgItem_QListView::repaintItem()
{
    KDlgItem_Base::repaintItem();

    QListView *itm = (QListView*)widget();
}
