/***************************************************************************
                          kdlgproplvis.cpp
                             -------------------
    begin                : Wed Mar 17 1999
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


#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtoolbutton.h>
#include <qgroupbox.h>
#include <krestrictedline.h>
#include <kcolorbtn.h>
#include <kfiledialog.h>
#include <kfontdialog.h>
#include <klocale.h>
#include "kdlgedit.h"
#include "kdlgeditwidget.h"
#include "kdlgitembase.h"
#include "kdlgitems.h"
#include "kdlgpropertybase.h"
#include "kdlgproplvis.h"
#include "items.h"
// for KDlgItemsIsValueTrue:
#include "itemsglobal.h"


AdvLvi_Base::AdvLvi_Base(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : QWidget( parent, name )
{
  dlgedit = dlged;
  setGeometry(0,0,0,0);
  propEntry = dpe;
  setBackgroundColor( colorGroup().base() );
  setFocusPolicy(StrongFocus);
  setEnabled(true);
}

#if 0
void AdvLvi_Base::paintEvent ( QPaintEvent * e )
{
    //  setBackgroundColor( colorGroup().base() );
  QWidget::paintEvent( e );
}
#endif

void AdvLvi_Base::refreshItem()
{
  KDlgItem_Base *selit = dlgedit->kdlg_get_edit_widget()->selectedWidget();
  if (selit)
      //    ((KDlgItem_QWidget*)selit)->repaintItem((QFrame*)selit->getItem());
      selit->repaintItem();

  if (propEntry->name.upper() == "NAME")
    dlgedit->kdlg_get_items_view()->refreshList();

  dlgedit->kdlg_get_edit_widget()->setModified(true);
}

AdvLvi_String::AdvLvi_String(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  leInput = new QLineEdit( this );
  leInput->setText(dpe->value);
  //  connect(leInput, SIGNAL(textChanged (const QString &)), SLOT(returnPressed()));
  connect(leInput, SIGNAL(returnPressed()), SLOT(returnPressed()));
}

QString AdvLvi_String::getText()
{
  if (leInput)
    return leInput->text();
  else
    return QString();
}

void AdvLvi_String::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (leInput)
    leInput->setGeometry(0,0,width(),height()+1);
}

void AdvLvi_String::returnPressed()
{
  propEntry->value = leInput->text();
  refreshItem();
}


AdvLvi_Int::AdvLvi_Int(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);

  leInput = new KRestrictedLine( this, 0, " 01234567890-" );
  leInput->setText(dpe->value.stripWhiteSpace());

  connect(leInput, SIGNAL(textChanged (const QString &)), SLOT(returnPressed()));
  connect(leInput, SIGNAL(returnPressed()), SLOT(returnPressed()));

  up = new QPushButton("",this);
  up->setAutoRepeat(true);
  down = new QPushButton("",this);
  down->setAutoRepeat(true);
  connect(up, SIGNAL(clicked()), SLOT(valInc()));
  connect(down, SIGNAL(clicked()), SLOT(valDec()));
}

void AdvLvi_Int::valInc()
{
  QString val = leInput->text();

  int dest = 0;

  if (val.length() != 0)
    {
      bool ok = true;
      dest = val.toInt(&ok);

      if (!ok)
        dest = 0;
    }

  dest++;
  leInput->setText(QString().setNum(dest));

}

void AdvLvi_Int::valDec()
{
  QString val = leInput->text();

  int dest = 0;

  if (val.length() != 0)
    {
      bool ok = true;
      dest = val.toInt(&ok);

      if (!ok)
        dest = 0;
    }

  dest--;
  leInput->setText(QString().setNum(dest));
}


QString AdvLvi_Int::getText()
{
  if (leInput)
    {
      return leInput->text();
    }
  else
    return QString();
}

void AdvLvi_Int::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (leInput)
    leInput->setGeometry(0,0,width()-15,height()+1);

  if (up)
    up->setGeometry(width()-15,0,15,(int)(height()/2));

  if (down)
    down->setGeometry(width()-15,(int)(height()/2)+1,15,(int)(height()/2));
}

void AdvLvi_Int::returnPressed()
{
  propEntry->value = leInput->text();
  refreshItem();
}

AdvLvi_UInt::AdvLvi_UInt(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);

  leInput = new KRestrictedLine( this, 0, " 01234567890" );
  leInput->setText(dpe->value.stripWhiteSpace());

  connect(leInput, SIGNAL(textChanged (const char QString &)), SLOT(returnPressed()));
  connect(leInput, SIGNAL(returnPressed()), SLOT(returnPressed()));

  up = new QPushButton("",this);
  up->setAutoRepeat(true);
  down = new QPushButton("",this);
  down->setAutoRepeat(true);
  connect(up, SIGNAL(clicked()), SLOT(valInc()));
  connect(down, SIGNAL(clicked()), SLOT(valDec()));
}

void AdvLvi_UInt::valInc()
{
  QString val = leInput->text();

  int dest = 0;

  if (val.length() != 0)
    {
      bool ok = true;
      dest = val.toInt(&ok);

      if (!ok)
        dest = 0;
    }

  dest++;
  leInput->setText(QString().setNum(dest));

}

void AdvLvi_UInt::valDec()
{
  QString val = leInput->text();

  int dest = 0;

  if (val.length() != 0)
    {
      bool ok = true;
      dest = val.toInt(&ok);

      if (!ok)
        dest = 0;
    }

  dest--;
  if(dest== -1){ // no unsigned it?
      dest=0;
  }
  leInput->setText(QString().setNum(dest));
}


QString AdvLvi_UInt::getText()
{
  if (leInput)
    {
      return leInput->text();
    }
  else
    return QString();
}

void AdvLvi_UInt::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (leInput)
    leInput->setGeometry(0,0,width()-15,height()+1);

  if (up)
    up->setGeometry(width()-15,0,15,(int)(height()/2));

  if (down)
    down->setGeometry(width()-15,(int)(height()/2)+1,15,(int)(height()/2));
}

void AdvLvi_UInt::returnPressed()
{
  propEntry->value = leInput->text();
  refreshItem();
}



AdvLvi_ExtEdit::AdvLvi_ExtEdit(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  btnMore = new QPushButton("...",this);
  leInput = new QLineEdit( this );
  leInput->setText(dpe->value);
  connect(leInput, SIGNAL(textChanged (const QString &)), SLOT(returnPressed()));
  connect(leInput, SIGNAL(returnPressed()), SLOT(returnPressed()));
}

QString AdvLvi_ExtEdit::getText()
{
  if (leInput)
    return leInput->text();
  else
    return QString();
}

void AdvLvi_ExtEdit::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );
  if (btnMore)
    btnMore->setGeometry(width()-15,0,15,height());

  if (leInput)
    leInput->setGeometry(0,0,width()-15,height()+1);
}

void AdvLvi_ExtEdit::returnPressed()
{
  propEntry->value = leInput->text();
  refreshItem();
}


AdvLvi_Filename::AdvLvi_Filename(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_ExtEdit( parent, dlged, dpe, name )
{
  connect( btnMore, SIGNAL( clicked() ), this, SLOT( btnPressed() ) );
}

void AdvLvi_Filename::btnPressed()
{
  QString fname = KFileDialog::getOpenFileName();
  if (!fname.isNull())
    {
      leInput->setText(fname);
    }
}



AdvLvi_Bool::AdvLvi_Bool(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  cbBool = new QComboBox( this );
  cbBool->insertItem(i18n("(not defined)"));
  cbBool->insertItem("true");
  cbBool->insertItem("false");

  cbBool->setCurrentItem(0);

  if (!dpe->value.isEmpty())
    {
      if (KDlgItemsIsValueTrue(dpe->value)==0)
          cbBool->setCurrentItem(2);
      else if (KDlgItemsIsValueTrue(dpe->value)==1)
          cbBool->setCurrentItem(1);
    }
  connect(cbBool, SIGNAL(activated(const QString &)), SLOT(activated(const QString &)));
}

void AdvLvi_Bool::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (cbBool)
    cbBool->setGeometry(0,0,width(),height()+1);
}

QString AdvLvi_Bool::getText()
{
  if (cbBool)
    {
      if (cbBool->currentItem()>0)
        return cbBool->currentText();
      else return QString("");
    }
  else
    return QString();
}

void AdvLvi_Bool::activated( const QString &s )
{
  propEntry->value = getText();
  refreshItem();
}


AdvLvi_Orientation::AdvLvi_Orientation(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  cbOrientation = new QComboBox( FALSE, this );
  cbOrientation->insertItem("Horizontal");
  cbOrientation->insertItem("Vertical");
  if(dpe->value == "Horizontal"){
    cbOrientation->setCurrentItem(0);
  }
  else{
    cbOrientation->setCurrentItem(1);
  }
  connect(cbOrientation, SIGNAL(activated(const QString &)), SLOT(activated(const QString &)));
}

void AdvLvi_Orientation::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (cbOrientation)
    cbOrientation->setGeometry(0,0,width(),height()+1);
}

QString AdvLvi_Orientation::getText()
{
  if (cbOrientation)
    return cbOrientation->currentText();
  else
    return QString();
}

void AdvLvi_Orientation::activated( const QString &s )
{
  propEntry->value = getText();
  refreshItem();
}


AdvLvi_ColorEdit::AdvLvi_ColorEdit(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  btn = new KColorButton(this);
  btn->setColor(Str2Color(dpe->value));
  leInput = new QLineEdit(this);
  leInput->setMaxLength(8);
  if (!dpe->value.isEmpty())
    {
      char s[255];
      sprintf(s,"0x%.2x%.2x%.2x",btn->color().red(),btn->color().green(),btn->color().blue());
      leInput->setText(QString(s));
    }
  connect(btn, SIGNAL(changed(const QColor&)), SLOT(changed(const QColor&)));
  connect(leInput, SIGNAL(textChanged (const QString &)), SLOT(returnPressed()));
  connect(leInput, SIGNAL(returnPressed()), SLOT(returnPressed()));
}

void AdvLvi_ColorEdit::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (btn)
    btn->setGeometry(width()-height(),0,height(),height()+1);

  if (leInput)
    leInput->setGeometry(0,0,width()-15,height()+1);
}

QString AdvLvi_ColorEdit::getText()
{
  return leInput->text();
}

QString AdvLvi_intToHex(int i)
{
  char s[10];
  sprintf(s,"%.2x",i);
  return QString(s);
}

void AdvLvi_ColorEdit::changed ( const QColor &newColor )
{
  propEntry->value = "0x"+AdvLvi_intToHex(newColor.red())+AdvLvi_intToHex(newColor.green())+AdvLvi_intToHex(newColor.blue());
  leInput->setText(propEntry->value);
  leInput->setFocus();
  refreshItem();
}

void AdvLvi_ColorEdit::returnPressed()
{
  propEntry->value = leInput->text();
  btn->setColor(Str2Color(leInput->text()));
  refreshItem();
}


AdvLvi_Font::AdvLvi_Font(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_ExtEdit( parent, dlged, dpe, name )
{
  connect( btnMore, SIGNAL( clicked() ), this, SLOT( btnPressed() ) );
}

void AdvLvi_Font::btnPressed()
{
  QFont myFont;
  int res = KFontDialog::getFont( myFont );

  if (res)
    {
      QString st = QString("\"") + myFont.family()    + QString("\" \"") +
                              QString().setNum(myFont.pointSize()) + QString("\" \"") +
                              QString().setNum(myFont.weight())   + QString("\" \"") +
                              QString(myFont.bold() ? QString("TRUE") : QString("FALSE")) + QString("\"");
      leInput->setText(st);
      propEntry->value=st;
      refreshItem();
    }
}



AdvLvi_BgMode::AdvLvi_BgMode(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  cbBool = new QComboBox( FALSE, this );
  cbBool->insertItem(i18n("(not set)"));
  cbBool->insertItem("FixedColor");
  cbBool->insertItem("FixedPixmap");
  cbBool->insertItem("NoBackground");
  cbBool->insertItem("PaletteForeground");
  cbBool->insertItem("PaletteBackground");
  cbBool->insertItem("PaletteLight");
  cbBool->insertItem("PaletteMidlight");
  cbBool->insertItem("PaletteDark");
  cbBool->insertItem("PaletteMid");
  cbBool->insertItem("PaletteText");
  cbBool->insertItem("PaletteBase");
  cbBool->setCurrentItem(0);
  connect(cbBool, SIGNAL(activated(const QString &)), SLOT(activated(const QString &)));
}

void AdvLvi_BgMode::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (cbBool)
    cbBool->setGeometry(0,0,width(),height()+1);
}

QString AdvLvi_BgMode::getText()
{
  if (cbBool)
    {
      QString s = cbBool->currentItem() ? QString(cbBool->currentText()) : QString();
      propEntry->value = s;
      refreshItem();
      return s;
    }
  else
    return QString();
}

void AdvLvi_BgMode::activated( const QString &s )
{
  propEntry->value = getText();
  refreshItem();
}


AdvLvi_Cursor::AdvLvi_Cursor(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  int cnt = 1;

  cbBool = new QComboBox( TRUE, this );
  #define InsertItem(n) cbBool->insertItem(n); if (QString(n).lower() == dpe->value.lower()) cbBool->setCurrentItem(cnt); cnt++;
  if ((dpe->value.left(1) == "(") || (dpe->value.left(1) == "["))
    {
      cbBool->insertItem("");
    }
  else
    {
      cbBool->insertItem(dpe->value);
    }
  cbBool->setCurrentItem(0);
  InsertItem(i18n("(not set)"));
  InsertItem("[handCursor]");
  InsertItem("[arrowCursor]");
  InsertItem("[upArrowCursor]");
  InsertItem("[crossCursor]");
  InsertItem("[waitCursor]");
  InsertItem("[ibeamCursor]");
  InsertItem("[sizeVerCursor]");
  InsertItem("[sizeHorCursor]");
  InsertItem("[sizeBDiagCursor]");
  InsertItem("[sizeFDiagCursor]");
  InsertItem("[sizeAllCursor]");
  InsertItem("[blankCursor]");
  #undef InsertItem
//  cbBool->setCurrentItem(1);
  cbBool->setAutoCompletion ( true );
  cbBool->setInsertionPolicy ( QComboBox::NoInsertion );

  btnMore = new QPushButton("...",this);
  connect( btnMore, SIGNAL( clicked() ), this, SLOT( btnPressed() ) );
  connect(cbBool, SIGNAL(activated(const QString &)), SLOT(activated(const QString &)));
}

void AdvLvi_Cursor::btnPressed()
{
  QString fname = KFileDialog::getOpenFileName();
  if (!fname.isNull())
    {
      cbBool->changeItem(fname,0);
      cbBool->setCurrentItem(0);
      propEntry->value = fname;
      refreshItem();
    }
}

void AdvLvi_Cursor::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (cbBool)
    cbBool->setGeometry(0,0,width()-15,height()+1);

  if (btnMore)
    btnMore->setGeometry(width()-15,0,15,height()+1);
}

QString AdvLvi_Cursor::getText()
{
  if (cbBool)
    {
      if ((cbBool->currentText()[0]!='[') && (cbBool->currentText()[0]!='('))
        {
          cbBool->setCurrentItem(0);
          cbBool->changeItem(cbBool->currentText(),0);
        }

      QString s = cbBool->currentItem() != 1 ? QString(cbBool->currentText()) : QString("");
      propEntry->value = s;
      refreshItem();

      return s;
    }
  else
    return QString();
}


void AdvLvi_Cursor::activated( const QString &s )
{
  propEntry->value = getText();
}


AdvLvi_Varname::AdvLvi_Varname(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_String(parent, dlged, dpe, name)
{
  connect(leInput, SIGNAL(textChanged(const QString &)), this, SLOT(VarnameChanged()) );
}

void AdvLvi_Varname::VarnameChanged()
{
  dlgedit->kdlg_get_edit_widget()->setVarnameChanged(true);
}

AdvLvi_ComboList::AdvLvi_ComboList(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  cbOrientation = new QComboBox( FALSE, this );

  int i = 0;
  QString src = dpe->data;
  QString s;

  s = getLineOutOfString(src,i);
  while (!s.isNull())
    {
      cbOrientation->insertItem(s);
      if (s==dpe->value)
        cbOrientation->setCurrentItem(i);
      i++;
      s = getLineOutOfString(src,i);
    }

  connect(cbOrientation, SIGNAL(activated(const QString &)), SLOT(activated(const QString &)));
}

void AdvLvi_ComboList::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (cbOrientation)
    cbOrientation->setGeometry(0,0,width(),height()+1);
}

QString AdvLvi_ComboList::getText()
{
  if (cbOrientation)
    return cbOrientation->currentText();
  else
    return QString();
}

void AdvLvi_ComboList::activated( const QString &s )
{
  propEntry->value = getText();
  refreshItem();
}




AdvLvi_MultiString::AdvLvi_MultiString(QWidget *parent, KDlgEdit *dlged, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, dlged, dpe, name )
{
  setGeometry(0,0,0,0);
  btnMore = new QToolButton(this);
  btnMore->setTextLabel("[QStrings...]");
  btnMore->setUsesTextLabel(true);
  txt = dpe->value;
  connect( btnMore, SIGNAL( clicked() ), SLOT( btnPressed() ) );
}

QString AdvLvi_MultiString::getText()
{
  return txt;
}

void AdvLvi_MultiString::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );
  if (btnMore)
    btnMore->setGeometry(0,0,width(),height());
}

void AdvLvi_MultiString::btnPressed()
{
  dlgMultiLineEdit *dlg = new dlgMultiLineEdit(this);
  if (dlg->exec())
    {
      txt = dlg->getText();
    }
  delete dlg;

  propEntry->value = txt;
  refreshItem();
}



dlgMultiLineEdit::dlgMultiLineEdit(QWidget *parent, const char *name)
  : QDialog(parent,name,true)
{
  resize(300,225);
  setFixedSize(300,225);

  gbGroupBox = new QGroupBox(this);
  gbGroupBox->setGeometry(10,10,280,175);
  gbGroupBox->setTitle(i18n("List of QStrings"));

  mleStrings = new QMultiLineEdit(this);
  mleStrings->setGeometry(20,30,260,130);

  int i = 0;
  QString src = ((AdvLvi_MultiString*)parent)->getText();
  QString s;
  QString dst;

  s = getLineOutOfString(src,i,"\\n");
  while (!s.isNull())
    {
      dst = dst + s + "\n";
      i++;
      s = getLineOutOfString(src,i,"\\n");
    }

  mleStrings->setText( dst.left(dst.length()-1) );

  connect(mleStrings, SIGNAL(textChanged()), SLOT(textChanged()));

  lRowCnt = new QLabel(this);
  lRowCnt->setGeometry(20,160,200,20);
  i = mleStrings->numLines();
  if (mleStrings->text().isEmpty())
    i = 0;
  lRowCnt->setText(QString().setNum(i) + QString(" ") + QString(i18n("row(s) entered.")));

  btnCancel = new QPushButton(this);
  btnCancel->setGeometry(220,190,70,25);
  btnCancel->setText(i18n("&Cancel"));
  connect(btnCancel, SIGNAL(clicked()), SLOT(reject()));

  btnOk = new QPushButton(this);
  btnOk->setGeometry(145,190,70,25);
  btnOk->setText(i18n("&OK"));
  connect(btnOk, SIGNAL(clicked()), SLOT(accept()));
}

void dlgMultiLineEdit::textChanged( )
{
  int i = mleStrings->numLines();
  if (mleStrings->text().isEmpty())
    i = 0;

  lRowCnt->setText(QString().setNum(i) + QString(" ") + QString(i18n("row(s) entered.")));
}

QString dlgMultiLineEdit::getText()
{
   QString src = mleStrings->text();
   QString dst = "";
   int l = src.length();

   for (int i = 0; i<l; i++)
     {
       if (src.left(1) == "\n")
         dst = dst + "\\n";
       else
         dst = dst + src.left(1);

       src = src.right(src.length()-1);
     }

   while (dst.right(2) == "\\n")
     dst = dst.left(dst.length()-2);

   return dst;
}
