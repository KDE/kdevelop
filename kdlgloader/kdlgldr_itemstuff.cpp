/***************************************************************************
                          kdlgloader_itemstuff.cpp  -  description
                             -------------------
    begin                : Wed Jun 2 1999
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

#ifndef DISABLE_KDEWIDGETS
  #define ENABLE_KDEWIDGETS            // Disable this if you don't want to build a kde loader
                                       // but a loader only needing qt libs (not the kde ones).
#endif

#include "kdlgldr.h"
#include <qrect.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qfont.h>
#include <qcolor.h>
#include <qpalette.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlcdnumber.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qmultilinedit.h>
#include <qprogressbar.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qscrollbar.h>
#include <qgroupbox.h>
#include <qlistview.h>

#ifdef ENABLE_KDEWIDGETS
#include <qcombobox.h>
#include <kdatepik.h>
#include <kdatetbl.h>
#include <kcolorbtn.h>
#include <kled.h>
#include <kprogress.h>
#include <kkeydialog.h>
#include <krestrictedline.h>
#include <kseparator.h>
#endif


#define UNDEF -32766

int isValTrue(QString val, int defaultval = 1);
int Prop2Int(QString val, int defaultval = UNDEF);

int isValTrue(QString val, int defaultval )
{
  QString v(val.upper());

  if (v=="FALSE" || v=="0" || v=="NO" || v=="NULL")
    return 0;
  if (v=="TRUE" || v=="1" || v=="YES")
    return 1;

  return defaultval;
}

int Prop2Int(QString val, int defaultval)
{
  if (val.length() == 0)
    return defaultval;

  bool ok = true;
  int dest = val.toInt(&ok);

  return ok ? dest : defaultval;
}



QFont Str2Font(QString desc)
{
  QString name;
  int size;
  int thickness ;
  bool italic = false;
  QString dummy;

  desc = desc.right(desc.length()-1);
  desc = desc.left(desc.length()-1);

  name = desc.left(desc.find('\"'));
  desc = desc.right(desc.length()-desc.find('\"')-3);

  dummy = desc.left(desc.find('\"'));
  desc = desc.right(desc.length()-desc.find('\"')-3);

  size = Prop2Int(dummy);
  if (size == UNDEF)
    size = 12;

  dummy = desc.left(desc.find('\"'));
  desc = desc.right(desc.length()-desc.find('\"')-3);

  thickness = Prop2Int(dummy);
  if (thickness == UNDEF)
    thickness = 50;

  if (isValTrue(desc, -1) != -1)
    italic = isValTrue(desc) ? true : false;

  return QFont(name, size, thickness, italic);
}

long hex2long(QString dig)
{
  dig = dig.lower();
  int v = Prop2Int(dig,-1);
  if (v == -1)
    {
      v = 0;
      if (dig == "a") v = 10;
      else if (dig == "b") v = 11;
      else if (dig == "c") v = 12;
      else if (dig == "d") v = 13;
      else if (dig == "e") v = 14;
      else if (dig == "f") v = 15;
    }

  return v;
}

QColor Str2Color(QString desc)
{
  int a = hex2long(desc.mid(2,1));
  int b = hex2long(desc.mid(3,1));
  int c = hex2long(desc.mid(4,1));
  int d = hex2long(desc.mid(5,1));
  int e = hex2long(desc.mid(6,1));
  int f = hex2long(desc.mid(7,1));

//  long col = f+e*0x10+d*0x100+c*0x1000+b*0x10000+a*0x100000;

  return QColor(b+a*0x10, d+c*0x10, f+e*0x10);
}



QString getLineOutOfString(QString src, int ln, QString sep)
{
  QString s = src+sep;
  QString act = "";
  int cnt = 0;
  int savecnt = 5000;

  while ((!s.isEmpty()) && (savecnt-->0))
    {
      if (s.left(sep.length()) == sep)
        {
          if (cnt == ln)
            return act;
          else
            act = "";
          cnt++;
          s = s.right(s.length()-sep.length()+1);
        }
      else
        {
          act = act + s.left(1);
        }
      s = s.right(s.length()-1);
    }

  return QString();
}





long KDlgLoader::setProperty(QWidget* w, QString name, QString val, QString ctype)
{
  if (!w)
    {
      Return(KDLGERR_BAD_PARAMETER)
    }

  int i = 0;
  ctype = ctype.lower().stripWhiteSpace();
  if (ctype.isEmpty())
    Return(KDLGERR_SUCCESS)

  /*******
     General properties (QWidget)
                            *******/

  if (name == "varname")               // VarName
    {
      if (actdb)
        {
          actdb->setVarName(val);
        }
    }

  if (name == "name")                  // Name
    {
      if (actdb)
        {
          actdb->setName(val);
        }
    }

  if (name == "ishidden")              // IsHidden
    {
      if (isValTrue(val, -1) == 0)
        w->show();
      else if (isValTrue(val, -1) == 1)
        w->hide();
      Return(KDLGERR_SUCCESS)
    }

  if (name == "isenabled")             // IsEnabled
    {
      if (isValTrue(val, -1) == 0)
        w->setEnabled(false);
      else if (isValTrue(val, -1) == 1)
        w->setEnabled(true);
      Return(KDLGERR_SUCCESS)
    }

  if (name == "acceptsdrops")          // AcceptsDrops
    {
      if (isValTrue(val, -1) == 0)
        w->setAcceptDrops(false);
      else if (isValTrue(val, -1) == 1)
        w->setAcceptDrops(true);
      Return(KDLGERR_SUCCESS)
    }

  if (name == "hasfocus")              // HasFocus
    {
      if (isValTrue(val, -1) == 1)
        w->setFocus();
      Return(KDLGERR_SUCCESS)
    }

  if (name == "x")                     // X
    {
      i = Prop2Int(val);
      if (i != UNDEF)
      {
        QRect geo(w->geometry());
        w->setGeometry(i,geo.y(),geo.width(), geo.height());
      }
      Return(KDLGERR_SUCCESS)
    }

  if (name == "y")                     // Y
    {
      i = Prop2Int(val);
      if (i != UNDEF)
      {
        QRect geo(w->geometry());
        w->setGeometry(geo.x(),i,geo.width(), geo.height());
      }
      Return(KDLGERR_SUCCESS)
    }

  if (name == "width")                 // W
    {
      i = Prop2Int(val);
      if (i != UNDEF)
      {
        QRect geo(w->geometry());
        w->setGeometry(geo.x(),geo.y(),i, geo.height());
      }
      Return(KDLGERR_SUCCESS)
    }

  if (name == "height")                // H
    {
      i = Prop2Int(val);
      if (i != UNDEF)
      {
        QRect geo(w->geometry());
        w->setGeometry(geo.x(),geo.y(),geo.width(), i);
      }
      Return(KDLGERR_SUCCESS)
    }

  if (name == "bgpixmap")              // BgPixmap
    {
      w->setBackgroundPixmap(QPixmap(val));
      Return(KDLGERR_SUCCESS)
    }

  if (name == "maskpixmap")            // MaskPixmap
    {
      w->setMask(QBitmap(val));
      Return(KDLGERR_SUCCESS)
    }

  if (name == "minwidth")              // MinWidth
    {
      i = Prop2Int(val);
      if (i != UNDEF)
        w->setMinimumWidth(i);
      Return(KDLGERR_SUCCESS)
    }

  if (name == "minheight")             // MinHeight
    {
      i = Prop2Int(val);
      if (i != UNDEF)
        w->setMinimumHeight(i);
      Return(KDLGERR_SUCCESS)
    }

  if (name == "maxwidth")              // MaxWidth
    {
      i = Prop2Int(val);
      if (i != UNDEF)
        w->setMaximumWidth(i);
      Return(KDLGERR_SUCCESS)
    }

  if (name == "maxheight")             // MaxHeight
    {
      i = Prop2Int(val);
      if (i != UNDEF)
        w->setMaximumHeight(i);
      Return(KDLGERR_SUCCESS)
    }

  if (name == "isfixedsize")           // IsFixedSize
    {
      if (isValTrue(val, -1) == 1)
        w->setFixedSize(w->width(),w->height());
      Return(KDLGERR_SUCCESS)
    }

  if (name == "sizeincx")              // SizeIncX
    {
      i = Prop2Int(val);
      if (i != UNDEF)
        w->setSizeIncrement(i,w->sizeIncrement().height());
      Return(KDLGERR_SUCCESS)
    }

  if (name == "sizeincy")              // SizeIncY
    {
      i = Prop2Int(val);
      if (i != UNDEF)
        w->setSizeIncrement(w->sizeIncrement().width(),i);
      Return(KDLGERR_SUCCESS)
    }

  if (name == "font")                  // Font
    {
      w->setFont(Str2Font(val));
      Return(KDLGERR_SUCCESS)
    }

  if (name == "bgmode")                // BgMode
    {
      if (val.lower() == "fixedcolor")
        w->setBackgroundMode(QWidget::FixedColor);
      else if (val.lower() == "fixedpixmap")
        w->setBackgroundMode(QWidget::FixedPixmap);
      else if (val.lower() == "nobackground")
        w->setBackgroundMode(QWidget::NoBackground);
      else if (val.lower() == "paletteforeground")
        w->setBackgroundMode(QWidget::PaletteForeground);
      else if (val.lower() == "palettebackground")
        w->setBackgroundMode(QWidget::PaletteBackground);
      else if (val.lower() == "palettelight")
        w->setBackgroundMode(QWidget::PaletteLight);
      else if (val.lower() == "palettemidlight")
        w->setBackgroundMode(QWidget::PaletteMidlight);
      else if (val.lower() == "palettedark")
        w->setBackgroundMode(QWidget::PaletteDark);
      else if (val.lower() == "palettemid")
        w->setBackgroundMode(QWidget::PaletteMid);
      else if (val.lower() == "palettetext")
        w->setBackgroundMode(QWidget::PaletteText);
      else if (val.lower() == "palettebase")
        w->setBackgroundMode(QWidget::PaletteBase);
      Return(KDLGERR_SUCCESS)
    }

  if (name == "bgcolor")               // BgColor
    {
      w->setBackgroundColor(Str2Color(val));
      Return(KDLGERR_SUCCESS)
    }

  if (name == "bgpalcolor")            // BgPalColor
    {
      w->setPalette( QPalette (Str2Color(val)));
      Return(KDLGERR_SUCCESS)
    }

///  addProp("Cursor",             "",             "Appearance",     ALLOWED_CURSOR);
////  addProp("Name",               "NoName",       "General",        ALLOWED_STRING);


  if (ctype.left(1) == "k")             // if kde widget...
    {
      #ifdef ENABLE_KDEWIDGETS
      if (setProperty_kde(w,name,val,ctype) != KDLGERR_SUCCESS)   // try kde widget properties.
        if (setProperty_qt(w,name,val,ctype) != KDLGERR_SUCCESS)  // if failed try qt properties.
          Return(KDLGERR_UNKNOWN);
      #endif
    }
  else                                                            // if non-kde widget (qt)...
    {
      if (setProperty_qt(w,name,val,ctype) != KDLGERR_SUCCESS)    // try qt widget properties.
        if (setProperty_kde(w,name,val,ctype) != KDLGERR_SUCCESS) // if failed try kde properties.
          Return(KDLGERR_UNKNOWN);
    }

  Return(KDLGERR_SUCCESS)
}

/** method to set properties of non qt widgets. */
long KDlgLoader::setProperty_qt(QWidget* w, QString name, QString val, QString ctype)
{
  /*******
     Additional properties for all button classes.
                            *******/

  int i;

  if ((ctype == "qpushbutton") ||
      (ctype == "qcheckbox") ||
      (ctype == "kcolorbutton") ||
      (ctype == "qradiobutton"))
    {
      if (name == "text")              // Text
        {
          ((QButton*)w)->setText(val);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isautoresize")      // isAutoResize
        {
          if (isValTrue(val, -1) == 1)
            ((QButton*)w)->setAutoResize(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isautorepeat")      // isAutoRepeat
        {
          if (isValTrue(val, -1) == 1)
            ((QButton*)w)->setAutoRepeat(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "pixmap")            // Pixmap
        {
          ((QButton*)w)->setPixmap(QPixmap(val));
          Return(KDLGERR_SUCCESS)
        }
    }



  /*******
     Additional properties (QPushButton)
                            *******/

  if (ctype == "qpushbutton")
    {

      if (name == "isdefault")         // isDefault
        {
          if (isValTrue(val, -1) == 1)
            ((QPushButton*)w)->setDefault(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isautodefault")     // isAutoDefault
        {
          if (isValTrue(val, -1) == 1)
            ((QPushButton*)w)->setAutoDefault(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "istogglebutton")    // isToggleButton
        {
          if (isValTrue(val, -1) == 1)
            ((QPushButton*)w)->setToggleButton(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "istoggledon")       // isToggledOn
        {
          if (isValTrue(val, -1) == 1)
            ((QPushButton*)w)->setOn(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "ismenubutton")      // isMenuButton
        {
          if (isValTrue(val, -1) == 1)
            ((QPushButton*)w)->setIsMenuButton(true);
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (QLabel)
                            *******/

  if (ctype == "qlabel")
    {

      if (name == "text")              // Text
        {
          ((QLabel*)w)->setText(val);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isautoresize")      // isAutoResize
        {
          if (isValTrue(val, -1) == 1)
            ((QLabel*)w)->setAutoResize(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "margin")            // Margin
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QLabel*)w)->setMargin(i);
          Return(KDLGERR_SUCCESS)
        }
    }

  /*******
     Additional properties (QLineEdit)
                            *******/

  if (ctype == "qlineedit")
    {

      if (name == "text")              // Text
        {
          ((QLineEdit*)w)->setText(val);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "hasframe")          // hasFrame
        {
          if (isValTrue(val, -1) == 0)
            ((QLineEdit*)w)->setFrame(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "maxlength")         // MaxLength
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QLineEdit*)w)->setMaxLength(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "cursorposition")    // CursorPosition
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QLineEdit*)w)->setCursorPosition(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "istextselected")    // isTextSelected
        {
          if (isValTrue(val, -1) == 1)
            ((QLineEdit*)w)->selectAll();
          Return(KDLGERR_SUCCESS)
        }
    }

  /*******
     Additional properties (QLCDNumber)
                            *******/

  if (ctype == "qlcdnumber")
    {

      if (name == "value")             // Text
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QLCDNumber*)w)->display(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "numdigits")         // NumDigits
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QLCDNumber*)w)->setNumDigits(i);
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (QCheckBox)
                            *******/

  if (ctype == "qcheckbox")
    {

      if (name == "ischecked")         // isChecked
        {
          if (isValTrue(val, -1) == 1)
            ((QCheckBox*)w)->setChecked(true);
          Return(KDLGERR_SUCCESS)
        }
    }

  /*******
     Additional properties (QRadioButton)
                            *******/

  if (ctype == "qradiobutton")
    {

      if (name == "ischecked")         // isChecked
        {
          if (isValTrue(val, -1) == 1)
            ((QRadioButton*)w)->setChecked(true);
          Return(KDLGERR_SUCCESS)
        }
    }

  /*******
     Additional properties (QComboBox, KCombo)
                            *******/

  if ((ctype == "qcombobox") || (ctype == "kcombo"))
    {

      if (name == "isautoresize")      // isAutoResize
        {
          if (isValTrue(val, -1) == 1)
            ((QComboBox*)w)->setAutoResize(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "entries")         // Entries
        {
          int i = 0;
          QString src = val;
          QString s;
          QString dst;

          s = getLineOutOfString(src,i,"\\n");
          while (!s.isNull())
            {
              ((QComboBox*)w)->insertItem(s);
              i++;
              s = getLineOutOfString(src,i,"\\n");
            }
        }

      if (name == "currentitem")       // CurrentItem
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QComboBox*)w)->setCurrentItem(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "sizelimit")         // SizeLimit
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QComboBox*)w)->setSizeLimit(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "maxCount")          // MaxCount
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QComboBox*)w)->setSizeLimit(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isautocompletion")  // isAutoCompletion
        {
          if (isValTrue(val, -1) == 1)
            ((QComboBox*)w)->setAutoCompletion(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "edittext")          // EditText
        {
          ((QComboBox*)w)->setEditText((const char*)val);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "insertionpolicy")   // InsertionPolicy
        {
          if (val.lower() == "NoInsertion")
            ((QComboBox*)w)->setInsertionPolicy(QComboBox::NoInsertion);
          else if (val.lower() == "AtTop")
            ((QComboBox*)w)->setInsertionPolicy(QComboBox::AtTop);
          else if (val.lower() == "AtCurrent")
            ((QComboBox*)w)->setInsertionPolicy(QComboBox::AtCurrent);
          else if (val.lower() == "AtBottom")
            ((QComboBox*)w)->setInsertionPolicy(QComboBox::AtBottom);
          else if (val.lower() == "AfterCurrent")
            ((QComboBox*)w)->setInsertionPolicy(QComboBox::AfterCurrent);
          else if (val.lower() == "BeforeCurrent")
            ((QComboBox*)w)->setInsertionPolicy(QComboBox::BeforeCurrent);
          Return(KDLGERR_SUCCESS)
        }

        if (name == "bgcolor")               // BgColor
          {
            ((QComboBox*)w)->setBackgroundColor(Str2Color(val));
            Return(KDLGERR_SUCCESS)
          }

        if (name == "bgpalcolor")            // BgPalColor
          {
            ((QComboBox*)w)->setPalette( QPalette (Str2Color(val)));
            Return(KDLGERR_SUCCESS)
          }
        if (name == "font")                  // Font
          {
            ((QComboBox*)w)->setFont(Str2Font(val));
            Return(KDLGERR_SUCCESS)
          }
        if (name == "isenabled")             // IsEnabled
          {
            if (isValTrue(val, -1) == 0)
              ((QComboBox*)w)->setEnabled(false);
            else if (isValTrue(val, -1) == 1)
              ((QComboBox*)w)->setEnabled(true);
            Return(KDLGERR_SUCCESS)
          }
    }



  /*******
     Additional properties (QListBox)
                            *******/

  if (ctype == "qlistbox")
    {

      if (name == "isautoupdate")      // isAutoUpdate
        {
          if (isValTrue(val, -1) == 0)
            ((QListBox*)w)->setAutoUpdate(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "setfixedvisiblelines")  // setFixedVisibleLines
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QListBox*)w)->setFixedVisibleLines(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isautoscroll")      // isAutoScroll
        {
          if (isValTrue(val, -1) == 0)
            ((QListBox*)w)->setAutoScroll(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isautoscrollbar")   // isAutoScrollBar
        {
          if (isValTrue(val, -1) == 0)
            ((QListBox*)w)->setAutoScrollBar(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isbottomscrollbar")   // isAutoBottomScroll
        {
          if (isValTrue(val, -1) == 1)
            ((QListBox*)w)->setBottomScrollBar(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isdragselect")      // isDragSelect
        {
          if (isValTrue(val, -1) == 0)
            ((QListBox*)w)->setDragSelect(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "issmoothscrolling") // isSmoothScrolling
        {
          if (isValTrue(val, -1) == 0)
            ((QListBox*)w)->setSmoothScrolling(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "entries")         // Entries
        {
          int i = 0;
          QString src = val;
          QString s;
          QString dst;

          s = getLineOutOfString(src,i,"\\n");
          while (!s.isNull())
            {
              ((QListBox*)w)->insertItem(s);
              i++;
              s = getLineOutOfString(src,i,"\\n");
            }
        }
    }


  /*******
     Additional properties (QMultiLineEdit)
                            *******/

  if (ctype == "qmultilineedit")
    {

      if (name == "text")              // Text
        {
          ((QMultiLineEdit*)w)->setText(val);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isautoupdate")      // isAutoUpdate
        {
          if (isValTrue(val, -1) == 0)
            ((QMultiLineEdit*)w)->setAutoUpdate(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isreadonly")        // isReadOnly
        {
          if (isValTrue(val, -1) == 1)
            ((QMultiLineEdit*)w)->setReadOnly(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isoverwritemode")  // isOverwriteMode
        {
          if (isValTrue(val, -1) == 1)
            ((QMultiLineEdit*)w)->setOverwriteMode(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "istextselected")   // isTextSelected
        {
          if (isValTrue(val, -1) == 1)
            ((QMultiLineEdit*)w)->selectAll();
          Return(KDLGERR_SUCCESS)
        }

      if (name == "setfixedvisiblelines")  // setFixedVisibleLines
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QMultiLineEdit*)w)->setFixedVisibleLines(i);
          Return(KDLGERR_SUCCESS)
        }
    }

  /*******
     Additional properties (QProgressBar)
                            *******/

  if (ctype == "qprogressbar")
    {

      if (name == "totalsteps")        // TotalSteps
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QProgressBar*)w)->setTotalSteps(i);
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (QGroupBox)
                            *******/

  if (ctype == "qgroupbox")
    {

      if (name == "title")             // Title
        {
          ((QGroupBox*)w)->setTitle(val);
          Return(KDLGERR_SUCCESS)
        }
    }



  /*******
     Additional properties (QScrollBar)
                            *******/

  if (ctype == "qscrollbar")
    {

      if (name == "value")             // Value
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QScrollBar*)w)->setValue(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "minvalue")          // MinValue
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QScrollBar*)w)->setRange(i, ((QScrollBar*)w)->maxValue());
          Return(KDLGERR_SUCCESS)
        }

      if (name == "maxvalue")          // MaxValue
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QScrollBar*)w)->setRange(((QScrollBar*)w)->minValue(), i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "istracking")        // isTracking
        {
          if (isValTrue(val, -1) == 0)
            ((QScrollBar*)w)->setTracking(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "orientation")       // Orientation
        {
          if (val.left(1).lower() == "h")
            ((QScrollBar*)w)->setOrientation( QScrollBar::Horizontal );
          else if (val.left(1).lower() == "v")
            ((QScrollBar*)w)->setOrientation( QScrollBar::Vertical );
          Return(KDLGERR_SUCCESS)
        }
    }

  /*******
     Additional properties (QSlider)
                            *******/

  if (ctype == "qslider")
    {

      if (name == "value")             // Value
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QSlider*)w)->setValue(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "minvalue")          // MinValue
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QSlider*)w)->setRange(i, ((QSlider*)w)->maxValue());
          Return(KDLGERR_SUCCESS)
        }

      if (name == "maxvalue")          // MaxValue
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QSlider*)w)->setRange(((QSlider*)w)->minValue(), i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "istracking")        // isTracking
        {
          if (isValTrue(val, -1) == 0)
            ((QSlider*)w)->setTracking(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "orientation")       // Orientation
        {
          if (val.left(1).lower() == "h")
            ((QSlider*)w)->setOrientation( QSlider::Horizontal );
          else if (val.left(1).lower() == "v")
            ((QSlider*)w)->setOrientation( QSlider::Vertical );
          Return(KDLGERR_SUCCESS)
        }
    }

  /*******
     Additional properties (QSpinbox)
                            *******/

  if (ctype == "qspinbox")
    {

      if (name == "value")             // Value
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QSpinBox*)w)->setValue(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "minvalue")          // MinValue
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QSpinBox*)w)->setRange(i, ((QSpinBox*)w)->maxValue());
          Return(KDLGERR_SUCCESS)
        }

      if (name == "maxvalue")          // MaxValue
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QSpinBox*)w)->setRange(((QSpinBox*)w)->minValue(), i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "iswrapping")        // isWrapping
        {
          if (isValTrue(val, -1) == 1)
            ((QSpinBox*)w)->setWrapping(true);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "prefix")            // Prefix
        {
          ((QSpinBox*)w)->setPrefix((const char*)val);
        }

      if (name == "suffix")            // Suffix
        {
          ((QSpinBox*)w)->setSuffix((const char*)val);
        }

      if (name == "specialvaltext")    // SpecialValText
        {
          ((QSpinBox*)w)->setSpecialValueText((const char*)val);
        }
    }


  /*******
     Additional properties (QListView)
                            *******/

  if (ctype == "qlistview")
    {

      if (name == "treestepsize")      // TreeStepSize
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QListView*)w)->setTreeStepSize(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "itemmargin")        // ItemMargin
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((QListView*)w)->setItemMargin(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "ismultiselection")  // isMultiSelection
        {
          if (isValTrue(val, -1) == 1)
            ((QListView*)w)->setMultiSelection(true);
          else if (isValTrue(val, -1) == 0)
            ((QListView*)w)->setMultiSelection(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isallcolumnsshowfocus")  // isAllColumnsShowFocus
        {
          if (isValTrue(val, -1) == 1)
            ((QListView*)w)->setAllColumnsShowFocus(true);
          else if (isValTrue(val, -1) == 0)
            ((QListView*)w)->setAllColumnsShowFocus(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isrootdecorated")   // isRootDecorated
        {
          if (isValTrue(val, -1) == 1)
            ((QListView*)w)->setRootIsDecorated(true);
          else if (isValTrue(val, -1) == 0)
            ((QListView*)w)->setRootIsDecorated(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "listviewfont")      // ListViewFont
        {
          ((QListView*)w)->setFont(Str2Font(val));
          Return(KDLGERR_SUCCESS)
        }

      if (name == "listviewpalette")   // ListViewPalette
        {
          ((QListView*)w)->setPalette(QPalette(Str2Color(val)));
          Return(KDLGERR_SUCCESS)
        }

      if (name == "vscrollbarmode")    // vScrollBarMode
        {
          if (val.lower() == "auto")
            ((QListView*)w)->setVScrollBarMode(QListView::Auto);
          else if (val.lower() == "alwaysoff")
            ((QListView*)w)->setVScrollBarMode(QListView::AlwaysOff);
          else if (val.lower() == "alwayson")
            ((QListView*)w)->setVScrollBarMode(QListView::AlwaysOn);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "hscrollbarmode")    // hScrollBarMode
        {
          if (val.lower() == "auto")
            ((QListView*)w)->setHScrollBarMode(QListView::Auto);
          else if (val.lower() == "alwaysoff")
            ((QListView*)w)->setHScrollBarMode(QListView::AlwaysOff);
          else if (val.lower() == "alwayson")
            ((QListView*)w)->setHScrollBarMode(QListView::AlwaysOn);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "columns")         // Columns
        {
          int i = 0;
          QString src = val;
          QString s;
          QString dst;

          s = getLineOutOfString(src,i,"\\n");
          while (!s.isNull())
            {
              if (!s.isEmpty())
                ((QListView*)w)->addColumn(s);
              i++;
              s = getLineOutOfString(src,i,"\\n");
            }
        }

      if (name == "entries")         // Entries
        {
          int i = 0;
          QString src = val;
          QString s;
          QString dst;

          s = getLineOutOfString(src,i,"\\n");
          while (!s.isNull())
            {
              new QListViewItem((QListView*)w, s);
              i++;
              s = getLineOutOfString(src,i,"\\n");
            }
        }
    }

   Return(KDLGERR_UNKNOWN)
}


/** method to set properties of kde widgets */
long KDlgLoader::setProperty_kde(QWidget* w, QString name, QString val, QString ctype)
{
#ifdef ENABLE_KDEWIDGETS

  /*******
     Additional properties (KCombo)
                            *******/

  int i;

  if (ctype == "kcombo")
    {
      if (name == "text")              // Text
        {
          ((QComboBox*)w)->setEditText(val);
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (KDatePicker)
                            *******/

  if (ctype == "kdatepicker")
    {
      if (name == "setdate")           // SetDate
        {
          if (val.length() == 10)
            {
              int y = Prop2Int(val.left(4),1999);
              int m = Prop2Int(val.mid(6,2),1);
              int d = Prop2Int(val.right(2),1);
              ((KDatePicker*)w)->setDate(QDate(y,m,d));
            }
          Return(KDLGERR_SUCCESS)
        }

      if (name == "fontsize")          // FontSize
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((KDatePicker*)w)->setFontSize(i);
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (KDateTable
                            *******/

  if (ctype == "kdatetable")
    {
      if (name == "setdate")           // SetDate
        {
          if (val.length() == 10)
            {
              int y = Prop2Int(val.left(4),1999);
              int m = Prop2Int(val.mid(6,2),1);
              int d = Prop2Int(val.right(2),1);
              ((KDateTable*)w)->setDate(QDate(y,m,d));
            }
          Return(KDLGERR_SUCCESS)
        }

      if (name == "fontsize")          // FontSize
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((KDateTable*)w)->setFontSize(i);
          Return(KDLGERR_SUCCESS)
        }
    }

  /*******
     Additional properties (KColorButton)
                            *******/

  if (ctype == "kcolorbutton")
    {

      if (name == "displayedcolor")    // DisplayedColor
        {
          ((KColorButton*)w)->setColor(Str2Color(val));
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (KLed)
                            *******/

  if (ctype == "kled")
    {

      if (name == "ledcolor")          // LedColor
        {
          if (val.lower() == "yellow")
            ((KLed*)w)->setColor(Qt::yellow);
          else if (val.lower() == "green")
            ((KLed*)w)->setColor(Qt::green);
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (KProgress)
                            *******/

  if (ctype == "kprogress")
    {

      if (name == "value")             // Value
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((KProgress*)w)->setValue(i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "minvalue")          // MinValue
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((KProgress*)w)->setRange(i, ((KProgress*)w)->maxValue());
          Return(KDLGERR_SUCCESS)
        }

      if (name == "maxvalue")          // MaxValue
        {
          i = Prop2Int(val);
          if (i != UNDEF)
            ((KProgress*)w)->setRange(((KProgress*)w)->minValue(), i);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "orientation")       // Orientation
        {
          if (val.left(1).lower() == "h")
            ((KProgress*)w)->setOrientation( KProgress::Horizontal );
          else if (val.left(1).lower() == "v")
            ((KProgress*)w)->setOrientation( KProgress::Vertical );
          Return(KDLGERR_SUCCESS)
        }

      if (name == "barcolor")          // BarColor
        {
          ((KProgress*)w)->setBarColor(Str2Color(val));
          Return(KDLGERR_SUCCESS)
        }

      if (name == "barpixmap")         // BarPixmap
        {
          ((KProgress*)w)->setBarPixmap(QPixmap(val));
          Return(KDLGERR_SUCCESS)
        }

      if (name == "istextenable")      // isTextEnable
        {
          if (isValTrue(val, -1) == 1)
            ((KProgress*)w)->setTextEnabled(true);
          else if (isValTrue(val, -1) == 0)
            ((KProgress*)w)->setTextEnabled(false);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "barstyle")          // BarStyle
        {
          if (val.lower() == "solid")
            ((KProgress*)w)->setBarStyle(KProgress::Solid);
          else if (val.lower() == "blocked")
            ((KProgress*)w)->setBarStyle(KProgress::Blocked);
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (KKeyButton)
                            *******/

  if (ctype == "kkeybutton")
    {
      if (name == "text")              // Text
        {
          ((KKeyButton*)w)->setText(val);
          Return(KDLGERR_SUCCESS)
        }

      if (name == "isedit")            // isEdit
        {
          if (isValTrue(val, -1) == 1)
            ((KKeyButton*)w)->setEdit(true);
          else if (isValTrue(val, -1) == 0)
            ((KKeyButton*)w)->setEdit(false);
          Return(KDLGERR_SUCCESS)
        }
    }


  /*******
     Additional properties (KSeparator)
                            *******/

  if (ctype == "kseparator")
    {
      if (name == "orientation")       // Orientation
        {
          if (val.left(1).lower() == "h")
            ((KSeparator*)w)->setOrientation( KSeparator::HLine );
          else if (val.left(1).lower() == "v")
            ((KSeparator*)w)->setOrientation( KSeparator::VLine );
          Return(KDLGERR_SUCCESS)
        }
    }

#endif
   Return(KDLGERR_UNKNOWN)
}



QWidget* KDlgLoader::createItem(QWidget*par, QString ctype)
{
#ifdef dlgldr_wth
  WatchMsg("[createItem()]");
#endif
  ctype = ctype.lower().stripWhiteSpace();

  if (ctype == "qwidget")
    return (QWidget*)new QWidget(par);
  else if (ctype == "qpushbutton")
    return (QWidget*)new QPushButton(par);
  else if (ctype == "qlabel")
    return (QWidget*)new QLabel(par);
  else if (ctype == "qlineedit")
    return (QWidget*)new QLineEdit(par);
  else if (ctype == "qlcdnumber")
    return (QWidget*)new QLCDNumber(par);
  else if (ctype == "qcheckbox")
    return (QWidget*)new QCheckBox(par);
  else if (ctype == "qradiobutton")
    return (QWidget*)new QRadioButton(par);
  else if (ctype == "qcombobox")
    return (QWidget*)new QComboBox(par);
  else if (ctype == "qlistbox")
    return (QWidget*)new QListBox(par);
  else if (ctype == "qmultilineedit")
    return (QWidget*)new QMultiLineEdit(par);
  else if (ctype == "qprogressbar")
    return (QWidget*)new QProgressBar(par);
  else if (ctype == "qspinbox")
    return (QWidget*)new QSpinBox(par);
  else if (ctype == "qslider")
    return (QWidget*)new QSlider(QSlider::Horizontal, par);
  else if (ctype == "qscrollbar")
    return (QWidget*)new QScrollBar(QScrollBar::Horizontal, par);
  else if (ctype == "qgroupbox")
    return (QWidget*)new QGroupBox(par);
  else if (ctype == "qlistview")
    return (QWidget*)new QListView(par);
#ifdef ENABLE_KDEWIDGETS
  else if (ctype == "kcombo")
    return (QWidget*)new QComboBox(par);
  else if (ctype == "kdatepicker")
    return (QWidget*)new KDatePicker(par);
  else if (ctype == "kdatetable")
    return (QWidget*)new KDateTable(par);
  else if (ctype == "kcolorbutton")
    return (QWidget*)new KColorButton(par);
  else if (ctype == "kled")
    return (QWidget*)new KLed(KLed::green, par);
  else if (ctype == "kprogress")
    return (QWidget*)new KProgress(par);
  else if (ctype == "kkeybutton")
    return (QWidget*)new KKeyButton("keybutton",par);
  else if (ctype == "krestrictedline")
    return (QWidget*)new KRestrictedLine(par);
  else if (ctype == "kseparator")
    return (QWidget*)new KSeparator(par);
#endif
  else
    return 0;
}


