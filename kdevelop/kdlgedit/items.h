/***************************************************************************
                           item.h  -  description
                             -------------------
    begin                : Thu Apr 08 1999
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


#ifndef _ITEMS_H_
#define _ITEMS_H_

#include <qframe.h>
#include <kcursor.h>
#include <kruler.h>
#include <kpopmenu.h>
#include "itemsglobal.h"
#include "kdlgeditwidget.h"
#include "kdlgpropwidget.h"
#include "kdlgpropertybase.h"
#include "kdlgitembase.h"


/**
 * This class is inherited from KDlgItem_Base. It defines an item using some #defines and
 * and including the item_class.cpp.inc file which contains the definition.
*/
class KDlgItem_QWidget : public KDlgItem_Base
{
  Q_OBJECT

public:
    KDlgItem_QWidget( KDlgEditWidget* editwid , QWidget *parent, bool ismainwidget, const char* name = 0 );

  #define CLASSNAME KDlgItem_QWidget       // the classname
  #define WIDGETTYPE QFrame               // type of the items widget
  #define CLASSDESC "QWidget"             // string returned by itemClass()
  #define ISCONTAINER
  #include "item_class.cpp.inc"           // includes the stuff
};


#include <qlineedit.h>
class KDlgItem_QLineEdit : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QLineEdit
  #define WIDGETTYPE QLineEdit
  #define CLASSDESC "QLineEdit"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};


#include <qpushbutton.h>
class KDlgItem_QPushButton : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QPushButton
  #define WIDGETTYPE QPushButton
  #define CLASSDESC "QPushButton"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};


#include <qlabel.h>
class KDlgItem_QLabel : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QLabel
  #define WIDGETTYPE QLabel
  #define CLASSDESC "QLabel"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qlcdnumber.h>
class KDlgItem_QLCDNumber : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QLCDNumber
  #define WIDGETTYPE QLCDNumber
  #define CLASSDESC "QLCDNumber"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qradiobutton.h>
class KDlgItem_QRadioButton : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QRadioButton
  #define WIDGETTYPE QRadioButton
  #define CLASSDESC "QRadioButton"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qcheckbox.h>
class KDlgItem_QCheckBox : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QCheckBox
  #define WIDGETTYPE QCheckBox
  #define CLASSDESC "QCheckBox"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qcombobox.h>
class KDlgItem_QComboBox : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QComboBox
  #define WIDGETTYPE QComboBox
  #define CLASSDESC "QComboBox"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qlistbox.h>
class KDlgItem_QListBox : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QListBox
  #define WIDGETTYPE QListBox
  #define CLASSDESC "QListBox"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qmultilinedit.h>
class KDlgItem_QMultiLineEdit : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QMultiLineEdit
  #define WIDGETTYPE QMultiLineEdit
  #define CLASSDESC "QMultiLineEdit"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qprogressbar.h>
class KDlgItem_QProgressBar : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QProgressBar
  #define WIDGETTYPE QProgressBar
  #define CLASSDESC "QProgressBar"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qspinbox.h>
#include <qlineedit.h>
class KDlgItem_QSpinBox : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QSpinBox
  #define WIDGETTYPE QSpinBox
  #define CLASSDESC "QSpinBox"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qslider.h>
class KDlgItem_QSlider : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QSlider
  #define WIDGETTYPE QSlider
  #define CLASSDESC "QSlider"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qscrollbar.h>
class KDlgItem_QScrollBar : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QScrollBar
  #define WIDGETTYPE QScrollBar
  #define CLASSDESC "QScrollBar"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qgroupbox.h>
class KDlgItem_QGroupBox : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QGroupBox
  #define WIDGETTYPE QGroupBox
  #define CLASSDESC "QGroupBox"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <qlistview.h>
class KDlgItem_QListView : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_QListView
  #define WIDGETTYPE QListView
  #define CLASSDESC "QListView"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <kcolorbtn.h>
class KDlgItem_KColorButton : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_KColorButton
  #define WIDGETTYPE KColorButton
  #define CLASSDESC "KColorButton"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <kdatepik.h>
class KDlgItem_KDatePicker : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_KDatePicker
  #define WIDGETTYPE KDatePicker
  #define CLASSDESC "KDatePicker"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <kdatetbl.h>
class KDlgItem_KDateTable : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_KDateTable
  #define WIDGETTYPE KDateTable
  #define CLASSDESC "KDateTable"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <kled.h>
class KDlgItem_KLed: public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_KLed
  #define WIDGETTYPE KLed
  #define CLASSDESC "KLed"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <kprogress.h>
class KDlgItem_KProgress : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_KProgress
  #define WIDGETTYPE KProgress
  #define CLASSDESC "KProgress"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <kkeydialog.h>
class KDlgItem_KKeyButton : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_KKeyButton
  #define WIDGETTYPE KKeyButton
  #define CLASSDESC "KKeyButton"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <krestrictedline.h>
class KDlgItem_KRestrictedLine : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_KRestrictedLine
  #define WIDGETTYPE KRestrictedLine
  #define CLASSDESC "KRestrictedLine"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};

#include <kseparator.h>
class KDlgItem_KSeparator : public KDlgItem_Base
{
  Q_OBJECT

  #define CLASSNAME KDlgItem_KSeparator
  #define WIDGETTYPE KSeparator
  #define CLASSDESC "KSeparator"
  #undef ISCONTAINER
  #include "item_class.cpp.inc"
};


#endif


