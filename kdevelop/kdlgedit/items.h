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


#ifndef KDLG_ITEMS_H
#define KDLG_ITEMS_H

#include <qframe.h>
#include "itemsglobal.h"
#include "kdlgpropwidget.h"
#include <kcursor.h>
#include "kdlgeditwidget.h"
#include <kruler.h>
#include <kpopmenu.h>

#ifndef DONTINC_ALL
  #define INC_LINEEDIT
  #define INC_PUSHBUTTON
  #define INC_LABEL
  #define INC_LCDNUMBER
  #define INC_RADIOBUTTON
  #define INC_CHECKBOX
  #define INC_COMBOBOX
  #define INC_LISTBOX
  #define INC_MULTILINEEDIT
  #define INC_PROGRESSBAR
  #define INC_SPINBOX
  #define INC_SLIDER
  #define INC_SCROLLBAR
  #define INC_GROUPBOX
  #define INC_LISTVIEW
  #define INC_KCOLORBUTTON
  #define INC_KCOMBO
  #define INC_KDATEPICKER
  #define INC_KDATETABLE

  #define INC_KLED
  #define INC_KLEDLAMP
  #define INC_KPROGRESS
  #define INC_KKEYBUTTON
  #define INC_KRESTRICTEDLINE
  #define INC_KTREELIST
  #define INC_KSEPARATOR
#endif

class KDlgEditWidget;

/**
  * @short Base item inherited by each complexer item.
  *
  * This is the base item inherited by each other item. It defines
  * the default methods and variables needed for each and every item.
  */
class KDlgItem_Base : public QObject
{
  Q_OBJECT
  public:
    /**
     * @param editwid The editwidget which creates the item.
     * @param parent The parent widget (used to create the child). Get it through the KDlgItem_Base::getItem() method of the parent items' class.
     * @param ismainwidget Is TRUE the ites cannot be moved and if it is resized, the rulers are also resized.
     * @param name Just passed to the items' widgets' constructor
    */
    KDlgItem_Base( KDlgEditWidget* editwid = 0, QWidget *parent = 0, bool ismainwidget = false, const char* name = 0 );
    virtual ~KDlgItem_Base() { }

    virtual void recreateItem();

    /**
     * Returns the type of the items class. Has to be overloaded in order to return the right type.
     * (i.e. returns "QPushButton" for a PushButton item.
    */
    virtual QString itemClass() { return QString("[Base]"); }

    /**
     * Returns a pointer to the items widget.
    */
    virtual QWidget *getItem() { return item; }

    /**
     * Rebuilds the item from its properties. If <i>it</i> is 0 the
     * idget stored in this class (<i>item</i>), otherwise
     * the one <i>it</i> points to is changed.
     * If you reimplement this method (and you should do so) you
     * can call repaintItem(item) in order to let your item be handled.
     * This makes sense since every widget in QT inherites QWidget so
     * you won't need to set the properties of the QWidget in your code.
    */
    virtual void repaintItem(QWidget *it = 0);

    /**
     * Returns a pointer to the properties of this item. See KDlgPropertyBase for
     * more details.
    */
    KDlgPropertyBase *getProps() { return props; }

    /**
     * Returns a pointer to the child database. If you call this method for
     * a KDlgItem_Widget you´ll get a pointer otherwise 0 because only a QWidget
     * may contain childs.
    */
    KDlgItemDatabase *getChildDb() { return childs; }

    /**
     * Returns the number child items if called for a KDlgItem_Widget otherwise 0.
    */
    int getNrOfChilds() { if (childs) return childs->numItems(); else return 0; }

    /**
     * Adds a child item to the children database.(Use only for KDlgItem_Widget's!!)<br><br>
     * Returns true if successful, otherwise false.
    */
    bool addChild(KDlgItem_Base *itm) { if (childs) return childs->addItem(itm); else return false; }

    /**
     * Returns a pointer to the KDlgEditWidget class which created this item.
    */
    KDlgEditWidget* getEditWidget() { return editWidget; }

    /**
     * Has to be overloaded ! Sets the state if this item to selected. (That means the border and the rectangles are painted)
    */
    virtual void select() { }

    /**
     * Has to be overloaded ! Sets the state if this item to not selected. (That means the border and the rectangles are NOT painted)
    */
    virtual void deselect() { }

    /**
     * Removes this item including all children (if a KDlgItem_Widget) from the dialog.
    */
    void deleteMyself();

    bool isMainWidget;
  protected:
    int Prop2Bool(QString name);
    int Prop2Int(QString name, int defaultval=0);
    QString Prop2Str(QString name);


    KDlgItemDatabase *childs;
    QWidget *item;
    KDlgPropertyBase *props;
    KDlgEditWidget* editWidget;
};


/**
 * This class is inherited from KDlgItem_Base. It defines an item using some #defines and
 * and including the item_class.cpp.inc file which contains the definition.
*/
class KDlgItem_Widget : public KDlgItem_Base
{
  Q_OBJECT

  public:
    KDlgItem_Widget( KDlgEditWidget* editwid , QWidget *parent, bool ismainwidget, const char* name = 0 );

  #define classname KDlgItem_Widget       // the classname
  #define widgettype QFrame               // type of the items widget
  #define classdesc "QWidget"             // string returned by itemClass()
  // use this macro to add lines to the MyWidget class of the class (see item_class.cpp.inc)
  #define MyWidgetAdd  public: MyWidget(KDlgItem_Widget* wid, QWidget* parent = 0, bool isMainWidget = false, const char* name = 0);
  #include "item_class.cpp.inc"           // includes the stuff
};


#ifdef INC_LINEEDIT
#include <qlineedit.h>
class KDlgItem_LineEdit : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_LineEdit
  #define widgettype QLineEdit
  #define classdesc "QLineEdit"
  #define MyWidgetAdd virtual void keyPressEvent ( QKeyEvent * ) {}
  #include "item_class.cpp.inc"
};
#endif


#ifdef INC_PUSHBUTTON
#include <qpushbutton.h>
class KDlgItem_PushButton : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_PushButton
  #define widgettype QPushButton
  #define classdesc "QPushButton"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif


#ifdef INC_LABEL
#include <qlabel.h>
class KDlgItem_Label : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_Label
  #define widgettype QLabel
  #define classdesc "QLabel"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif


#ifdef INC_LCDNUMBER
#include <qlcdnumber.h>
class KDlgItem_LCDNumber : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_LCDNumber
  #define widgettype QLCDNumber
  #define classdesc "QLCDNumber"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif


#ifdef INC_RADIOBUTTON
#include <qradiobutton.h>
class KDlgItem_RadioButton : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_RadioButton
  #define widgettype QRadioButton
  #define classdesc "QRadioButton"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif


#ifdef INC_CHECKBOX
#include <qcheckbox.h>
class KDlgItem_CheckBox : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_CheckBox
  #define widgettype QCheckBox
  #define classdesc "QCheckBox"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_COMBOBOX
#include <qcombobox.h>
class KDlgItem_ComboBox : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_ComboBox
  #define widgettype QComboBox
  #define classdesc "QComboBox"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_LISTBOX
#include <qlistbox.h>
class KDlgItem_ListBox : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_ListBox
  #define widgettype QListBox
  #define classdesc "QListBox"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_MULTILINEEDIT
#include <qmultilinedit.h>
class KDlgItem_MultiLineEdit : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_MultiLineEdit
  #define widgettype QMultiLineEdit
  #define classdesc "QMultiLineEdit"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_PROGRESSBAR
#include <qprogressbar.h>
class KDlgItem_ProgressBar : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_ProgressBar
  #define widgettype QProgressBar
  #define classdesc "QProgressBar"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_SPINBOX
#include <qspinbox.h>
class KDlgItem_SpinBox : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_SpinBox
  #define widgettype QSpinBox
  #define classdesc "QSpinBox"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_SLIDER
#include <qslider.h>
class KDlgItem_Slider : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_Slider
  #define widgettype QSlider
  #define classdesc "QSlider"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_SCROLLBAR
#include <qscrollbar.h>
class KDlgItem_ScrollBar : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_ScrollBar
  #define widgettype QScrollBar
  #define classdesc "QScrollBar"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_GROUPBOX
#include <qgroupbox.h>
class KDlgItem_GroupBox : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_GroupBox
  #define widgettype QGroupBox
  #define classdesc "QGroupBox"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_LISTVIEW
#include <qlistbox.h>
class KDlgItem_ListView : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_ListView
  #define widgettype QListBox
  #define classdesc "QListView"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif





#ifdef INC_KCOLORBUTTON
#include <kcolorbtn.h>
class KDlgItem_KColorButton : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KColorButton
  #define widgettype KColorButton
  #define classdesc "KColorButton"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KCOMBO
#include <kcombo.h>
class KDlgItem_KCombo : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KCombo
  #define widgettype KCombo
  #define classdesc "KCombo"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KDATEPICKER
#include <kdatepik.h>
class KDlgItem_KDatePicker : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KDatePicker
  #define widgettype KDatePicker
  #define classdesc "KDatePicker"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KDATETABLE
#include <kdatetbl.h>
class KDlgItem_KDateTable : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KDateTable
  #define widgettype KDateTable
  #define classdesc "KDateTable"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KLED
#include <kled.h>
class KDlgItem_KLed: public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KLed
  #define widgettype KLed
  #define classdesc "KLed"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KLEDLAMP
#include <kledlamp.h>
class KDlgItem_KLedLamp : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KLedLamp
  #define widgettype KLedLamp
  #define classdesc "KLedLamp"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KPROGRESS
#include <kprogress.h>
class KDlgItem_KProgress : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KProgress
  #define widgettype KProgress
  #define classdesc "KProgress"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KKEYBUTTON
#include <kkeydialog.h>
class KDlgItem_KKeyButton : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KKeyButton
  #define widgettype KKeyButton
  #define classdesc "KKeyButton"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KRESTRICTEDLINE
#include <krestrictedline.h>
class KDlgItem_KRestrictedLine : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KRestrictedLine
  #define widgettype KRestrictedLine
  #define classdesc "KRestrictedLine"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KTREELIST
#include <ktreelist.h>
class KDlgItem_KTreeList : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KTreeList
  #define widgettype KTreeList
  #define classdesc "KTreeList"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif

#ifdef INC_KSEPARATOR
#include <kseparator.h>
class KDlgItem_KSeparator : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_KSeparator
  #define widgettype KSeparator
  #define classdesc "KSeparator"
  #undef MyWidgetAdd
  #include "item_class.cpp.inc"
};
#endif




#endif


