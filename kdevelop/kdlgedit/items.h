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


#define DECLARE(classname) \
  public: \
      classname(KDlgEditWidget *editwid, KDlgItem_Base *parentit); \
      virtual ~classname() {} \
      virtual void repaintItem(); \
      virtual void addMyPropEntrys(); 


class KDlgItem_QWidget : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QWidget)
public:
  // Constructor for main widget:
  KDlgItem_QWidget(KDlgEditWidget *editwid, Role role, KDlgItem_Base *parentit);
};


#include <qlineedit.h>
class KDlgItem_QLineEdit : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QLineEdit)
};


#include <qpushbutton.h>
class KDlgItem_QPushButton : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QPushButton)
};


#include <qlabel.h>
class KDlgItem_QLabel : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QLabel)
};

#include <qlcdnumber.h>
class KDlgItem_QLCDNumber : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QLCDNumber)
};

#include <qradiobutton.h>
class KDlgItem_QRadioButton : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QRadioButton)
};

#include <qcheckbox.h>
class KDlgItem_QCheckBox : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QCheckBox)
};

#include <qcombobox.h>
class KDlgItem_QComboBox : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QComboBox)
};

#include <qlistbox.h>
class KDlgItem_QListBox : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QListBox)
};

#include <qmultilinedit.h>
class KDlgItem_QMultiLineEdit : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QMultiLineEdit)
};

#include <qprogressbar.h>
class KDlgItem_QProgressBar : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QProgressBar)
};

#include <qspinbox.h>
#include <qlineedit.h>
class KDlgItem_QSpinBox : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QSpinBox)
};

#include <qslider.h>
class KDlgItem_QSlider : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QSlider)
};

#include <qscrollbar.h>
class KDlgItem_QScrollBar : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QScrollBar)
};

#include <qgroupbox.h>
class KDlgItem_QGroupBox : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QGroupBox)
};

#include <qlistview.h>
class KDlgItem_QListView : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_QListView)
};

#include <kcolorbtn.h>
class KDlgItem_KColorButton : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_KColorButton)
};

#include <kdatepik.h>
class KDlgItem_KDatePicker : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_KDatePicker)
};

#include <kdatetbl.h>
class KDlgItem_KDateTable : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_KDateTable)
};

#include <kled.h>
class KDlgItem_KLed: public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_KLed)
};

#include <kprogress.h>
class KDlgItem_KProgress : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_KProgress)
};

#include <kkeydialog.h>
class KDlgItem_KKeyButton : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_KKeyButton)
};

#include <krestrictedline.h>
class KDlgItem_KRestrictedLine : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_KRestrictedLine)
};

#include <kseparator.h>
class KDlgItem_KSeparator : public KDlgItem_Base
{
  Q_OBJECT
  DECLARE(KDlgItem_KSeparator)
};


#endif
