/***************************************************************************
                         kdlgitembase.cpp  -  
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
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


#include <qpixmap.h>
#include <qbitmap.h>
#include <kapp.h>
#include <kpopmenu.h>
#include "kdlgitembase.h"
#include "kdlgpropertybase.h"
#include "kdlgeditwidget.h"
#include "itemsglobal.h"


KDlgItem_Base::KDlgItem_Base( KDlgEditWidget* editwid , QWidget *parent , bool ismainwidget, const char* name )
  : QObject(parent,name)
{
  editWidget = editwid;
  childs = 0;
  isMainWidget = ismainwidget;
  item = 0;
//  item = new QWidget(parent);
//  item->setMouseTracking(true);

  props = new KDlgPropertyBase();
  repaintItem();
}


void KDlgItem_Base::recreateItem()
{
}


QString KDlgItem_Base::itemClass()
{
    return QString("[Base]");   
}


int KDlgItem_Base::getNrOfChilds()
{
    if (childs)
	return childs->numItems();
    else
	return 0;
}


bool KDlgItem_Base::addChild(KDlgItem_Base *itm)
{
    if (childs)
	return childs->addItem(itm);
    else
	return false;
}


void KDlgItem_Base::deleteMyself()
{
  if (!childs)
    return;

  KDlgItem_Base *w = childs->getFirst();

  while (w)
  {
    w->deleteMyself();
    if (w->getItem())
      delete w->getItem();
    delete w;
    w = childs->getNext();
  }

  childs->clear();
}


int KDlgItem_Base::Prop2Bool(QString name)
{
  if (props->getProp(name))
    return KDlgItemsIsValueTrue( props->getProp(name)->value );
  else
    return -1;
}


int KDlgItem_Base::Prop2Int(QString name, int defaultval)
{
  if (props->getProp(name))
    return props->getIntFromProp(name,defaultval);
  else
    return defaultval;
}


QString KDlgItem_Base::Prop2Str(QString name)
{
  if (props->getProp(name))
    return props->getProp(name)->value;
  else
    return QString();
}


void KDlgItem_Base::repaintItem(QWidget *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  int b=0;
  int x=0,y=0;
  QString str;

  #define Prop2Bool(name) (KDlgItemsIsValueTrue( props->getProp(name)->value ))
  #define Prop2Str(name) (props->getProp(name)->value)
  #define setB2Prop(name) b = Prop2Bool(name);
  #define setStr2Prop(name) str = Prop2Str(name);
  #define ifBValid(name) setB2Prop(name); if ((b==0) || (b==1))
  #define ifBTrue(name) setB2Prop(name); if (b==1)
  #define ifBFalse(name) setB2Prop(name); if (b==0)
  #define strNotEmpty(str) (str.length()!=0)

  itm->setMinimumWidth(props->getIntFromProp("MinWidth",0));
  itm->setMaximumWidth(props->getIntFromProp("MaxWidth",32767));
  itm->setMinimumHeight(props->getIntFromProp("MinHeight",0));
  itm->setMaximumHeight(props->getIntFromProp("MaxHeight",32767));

/*
  QString val = Prop2Str("BgMode");
  if (val.lower() == "fixedcolor")
    itm->setBackgroundMode(QWidget::FixedColor);
  else if (val.lower() == "fixedpixmap")
    itm->setBackgroundMode(QWidget::FixedPixmap);
  else if (val.lower() == "nobackground")
    itm->setBackgroundMode(QWidget::NoBackground);
  else if (val.lower() == "paletteforeground")
    itm->setBackgroundMode(QWidget::PaletteForeground);
  else if (val.lower() == "palettebackground")
    itm->setBackgroundMode(QWidget::PaletteBackground);
  else if (val.lower() == "palettelight")
    itm->setBackgroundMode(QWidget::PaletteLight);
  else if (val.lower() == "palettemidlight")
    itm->setBackgroundMode(QWidget::PaletteMidlight);
  else if (val.lower() == "palettedark")
    itm->setBackgroundMode(QWidget::PaletteDark);
  else if (val.lower() == "palettemid")
    itm->setBackgroundMode(QWidget::PaletteMid);
  else if (val.lower() == "palettetext")
    itm->setBackgroundMode(QWidget::PaletteText);
  else if (val.lower() == "palettebase")
    itm->setBackgroundMode(QWidget::PaletteBase);
  else
    itm->setBackgroundMode(QWidget::PaletteBackground);
*/

  ifBTrue("IsFixedSize")
    itm->setFixedSize( props->getIntFromProp("Width",itm->width()),
                       props->getIntFromProp("Height",itm->height()) );

  x=props->getIntFromProp("SizeIncX",1);
  y=props->getIntFromProp("SizeIncY",1);

  if ((x!=1) || (y!=1))
    itm->setSizeIncrement(x,y);


  itm->setGeometry(isMainWidget ? RULER_WIDTH : (props->getIntFromProp("X",itm->x())),
                   isMainWidget ? RULER_HEIGHT : (props->getIntFromProp("Y",itm->y())),
                   props->getIntFromProp("Width",itm->width()),
                   props->getIntFromProp("Height",itm->height()));

  ifBTrue("IsHidden")
    itm->hide();
  else
    itm->show();

//  ifBValid("IsEnabled")
//    itm->setEnabled(b);

  setStr2Prop("BgPixmap");
  if (strNotEmpty(str))
    itm->setBackgroundPixmap( QPixmap( str ) );
  else
    itm->setBackgroundPixmap( QPixmap() );

  setStr2Prop("MaskBitmap");
  if (strNotEmpty(str))
    itm->setMask( QBitmap( str ) );
  else
    itm->setMask( QBitmap() );

  setStr2Prop("Font");
  if (strNotEmpty(str))
    {
    itm->setFont(KDlgItemsGetFont(str));
//    itm->setFont(KDlgItemsGetFont("\"helvetica\" \"20\" \"75\" \"TRUE\""));
    }
  else
    {
      QWidget wid;
      itm->setFont(wid.font());
    }

}


void KDlgItem_Base::execContextMenu(bool ismain)
{
    QString pixmapdir = KApplication::kde_toolbardir() + "/";
    QString kdevpixmapdir = KApplication::kde_datadir() + QString("/kdevelop/toolbar/");
    
#define mkQPixTb(fn) QPixmap(pixmapdir + fn)
#define mkQPixDd(fn) QPixmap(kdevpixmapdir + fn)

    KPopupMenu phelp;
    phelp.setTitle( itemClass() );
    if (!ismain)
        {
          phelp.insertItem( mkQPixTb("prev.xpm"), i18n("&Raise"),
			    getEditWidget(), SLOT(slot_raiseSelected()) );
          phelp.insertItem( mkQPixTb("next.xpm"), i18n("&Lower"),
			    getEditWidget(), SLOT(slot_lowerSelected()) );
          phelp.insertItem( mkQPixTb("top.xpm"), i18n("Raise to &top"),
			    getEditWidget(), SLOT(slot_raiseTopSelected()) );
          phelp.insertItem( mkQPixTb("bottom.xpm"), i18n("Lower to &bottom"),
			    getEditWidget(), SLOT(slot_lowerBottomSelected()) );
          phelp.insertSeparator();
          phelp.insertItem( mkQPixDd("cut.xpm"), i18n("C&ut"),
			    getEditWidget(), SLOT(slot_cutSelected()) );
          phelp.insertItem( mkQPixTb("delete.xpm"), i18n("&Delete"),
			    getEditWidget(), SLOT(slot_deleteSelected()) );
          phelp.insertItem( mkQPixDd("copy.xpm"), i18n("&Copy"),
			    getEditWidget(), SLOT(slot_copySelected()) );
        }
    phelp.insertItem( mkQPixDd("paste.xpm"), i18n("&Paste"),
		      getEditWidget(), SLOT(slot_pasteSelected()) );
    phelp.insertSeparator();
    phelp.insertItem( mkQPixTb("help.xpm"), i18n("&Help"),
		      getEditWidget(), SLOT(slot_helpSelected()) );
    phelp.exec(QCursor::pos());
}
