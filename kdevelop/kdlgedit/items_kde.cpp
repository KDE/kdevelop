/***************************************************************************
                             items_kde.cpp
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
 *  KColorButton                                                           *
 ***************************************************************************/


KDlgItem_KColorButton::KDlgItem_KColorButton(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new KColorButton(parentit->widget(), "KDlgEdit KColorButton"));
    addMyPropEntrys();
}

void KDlgItem_KColorButton::addMyPropEntrys()
{
  if (!props)
    return;
  props->addProp("Text",           "Button",       "General",        ALLOWED_STRING);
  props->addProp("isDefault",      "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoDefault" , "",             "General",        ALLOWED_BOOL);
  props->addProp("isToggleButton", "",             "General",        ALLOWED_BOOL);
  props->addProp("isToggledOn",    "",             "General",        ALLOWED_BOOL);
  props->addProp("isMenuButton",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoResize",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoRepeat",   "",             "General",        ALLOWED_BOOL);
  props->addProp("DisplayedColor", "",             "General",        ALLOWED_COLOR);
}

void KDlgItem_KColorButton::repaintItem()
{
    KDlgItem_Base::repaintItem();

    KColorButton *itm = (KColorButton*)widget();

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

  if (!Prop2Str("DisplayedColor").isNull())
    itm->setColor(Str2Color(Prop2Str("DisplayedColor")));
  else
    itm->setColor(QColor());
}


/***************************************************************************
 *  KDatePicker                                                            *
 ***************************************************************************/


KDlgItem_KDatePicker::KDlgItem_KDatePicker(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new KDatePicker(parentit->widget()));
    addMyPropEntrys();
}

void KDlgItem_KDatePicker::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("FontSize",   "",             "Appearance",        ALLOWED_INT);
  props->addProp("SetDate",    "",             "General",           ALLOWED_STRING);
}

void KDlgItem_KDatePicker::repaintItem()
{
    KDlgItem_Base::repaintItem();

    KDatePicker *itm = (KDatePicker*)widget();
}


/***************************************************************************
 *  KDateTable                                                             *
 ***************************************************************************/


KDlgItem_KDateTable::KDlgItem_KDateTable(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new KDateTable(parentit->widget()));
    addMyPropEntrys();
    repaintItem();
}


void KDlgItem_KDateTable::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("FontSize",   "",             "Appearance",        ALLOWED_INT);
  props->addProp("SetDate",    "",             "General",           ALLOWED_STRING);
}

void KDlgItem_KDateTable::repaintItem()
{
    KDlgItem_Base::repaintItem();
    
    KDateTable *itm = (KDateTable*)widget();
}


/***************************************************************************
 *  KLed                                                                   *
 ***************************************************************************/


KDlgItem_KLed::KDlgItem_KLed(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new KLed(KLed::green, parentit->widget(), "KDlgEdit KLed"));
    addMyPropEntrys();
}

void KDlgItem_KLed::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("LedColor",   "Yellow",           "Appearance",        ALLOWED_COMBOLIST,  "Yellow\nOrange\nRed\nGreen\nBlue"  );

}

void KDlgItem_KLed::repaintItem()
{
    KDlgItem_Base::repaintItem();

    KLed *itm = (KLed*)widget();

}


/***************************************************************************
 *  KProgress                                                             *
 ***************************************************************************/


KDlgItem_KProgress::KDlgItem_KProgress(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new KProgress(parentit->widget(), "KDlgEdit KProgress"));
    addMyPropEntrys();
}

void KDlgItem_KProgress::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Value",        "0",          "Appearance",        ALLOWED_INT );
  props->addProp("minValue",     "0",          "Appearance",        ALLOWED_INT );
  props->addProp("maxValue",     "100",        "Appearance",        ALLOWED_INT );
  props->addProp("BarStyle",     "Blocked",    "Appearance",        ALLOWED_COMBOLIST,  "Solid\nBlocked"  );
  props->addProp("Orientation",  "Horizontal", "Appearance",        ALLOWED_ORIENTATION );
  props->addProp("BarColor",     "",           "Appearance",        ALLOWED_COLOR );
  props->addProp("BarPixmap",    "",           "Appearance",        ALLOWED_FILE );
  props->addProp("isTextEnable", "",           "Appearance",        ALLOWED_BOOL );
}

void KDlgItem_KProgress::repaintItem()
{
    KDlgItem_Base::repaintItem();

    KProgress *itm = (KProgress*)widget();
}


/***************************************************************************
 *  KKeyButton                                                             *
 ***************************************************************************/


KDlgItem_KKeyButton::KDlgItem_KKeyButton(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new KKeyButton("K", parentit->widget()));
    addMyPropEntrys();
}

void KDlgItem_KKeyButton::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("isEdit",       "",                  "General",        ALLOWED_BOOL);
  props->addProp("Text",         "",                  "General",        ALLOWED_STRING);
}

void KDlgItem_KKeyButton::repaintItem()
{
    KDlgItem_Base::repaintItem();

    KKeyButton *itm = (KKeyButton*)widget();

    itm->setText(Prop2Str("Text"));
}


/***************************************************************************
 *  KRestrictedLine                                                        *
 ***************************************************************************/


KDlgItem_KRestrictedLine::KDlgItem_KRestrictedLine(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new KRestrictedLine(parentit->widget(), "KDlgEdit KRestrictedLine"));
    addMyPropEntrys();
}

void KDlgItem_KRestrictedLine::addMyPropEntrys()
{
  if (!props)
    return;

}

void KDlgItem_KRestrictedLine::repaintItem()
{
  KRestrictedLine *itm = (KRestrictedLine*)widget();

  KDlgItem_Base::repaintItem();
}


/***************************************************************************
 *  KSeparator                                                             *
 ***************************************************************************/


KDlgItem_KSeparator::KDlgItem_KSeparator(KDlgEditWidget *editwid, KDlgItem_Base *parentit)
    : KDlgItem_Base(editwid, parentit, Widget)
{
    setWidget(new KSeparator(parentit->widget(), "KDlgEdit KSeparator"));
    addMyPropEntrys();
}

void KDlgItem_KSeparator::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Orientation",           "Horizontal",       "General",        ALLOWED_ORIENTATION);
}

void KDlgItem_KSeparator::repaintItem()
{
    KDlgItem_Base::repaintItem();

    KSeparator *itm = (KSeparator*)widget();


  if(Prop2Str("Orientation") == "Horizontal"){
    itm->setOrientation(KSeparator::HLine);
  }
  else{
    itm->setOrientation(KSeparator::VLine);
  }
}
