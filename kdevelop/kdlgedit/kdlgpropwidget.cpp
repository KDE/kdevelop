/***************************************************************************
                          kdlgpropwidget.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by                          
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "kdlgpropwidget.h"
#include "kdlgproplvis.h"
#include <kcolorbtn.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <kfiledialog.h>
#include <krestrictedline.h>
#include <kspinbox.h>
#include <stdio.h>
#include "../ckdevelop.h"
#include "kdlgproplv.h"
#include "item_base.h"
#include "defines.h"


bool isValueTrue(QString val)
{
  QString v(val.upper());

  if (v=="FALSE" || v=="0" || v=="NO" || v=="NULL" || v.isEmpty())
    return false;

  return true;
}


AdvListView::AdvListView( QWidget * parent , const char * name )
  : QListView( parent, name )
{
}

void AdvListView::viewportMousePressEvent ( QMouseEvent *e )
{
  QListView::viewportMousePressEvent( e );
  updateWidgets();
}

void AdvListView::paintEvent ( QPaintEvent *e )
{
  QListView::paintEvent( e );
  updateWidgets();
}

void AdvListView::mousePressEvent ( QMouseEvent *e )
{
  QListView::mousePressEvent( e );
  updateWidgets();
}

void AdvListView::mouseMoveEvent ( QMouseEvent *e )
{
  QListView::mousePressEvent( e );
  updateWidgets();
}

void AdvListView::keyPressEvent ( QKeyEvent *e )
{
  QListView::keyPressEvent( e );
  updateWidgets();
}

void AdvListView::moveEvent ( QMoveEvent *e )
{
  QListView::moveEvent( e );
  updateWidgets();
}

void AdvListView::resizeEvent ( QResizeEvent *e )
{
  QListView::resizeEvent( e );
  updateWidgets();
}

void AdvListView::updateWidgets()
{
  AdvListViewItem* i = (AdvListViewItem*)firstChild();

  while (i) {
    i->updateWidgets();
    i = (AdvListViewItem*)i->nextSibling();
  }
}

/**
 *
*/
AdvListViewItem::AdvListViewItem( QListView * parent, QString a, QString b)
   : QListViewItem( parent, a, b )
{
  init();
//  parent->connect( parent, SIGNAL(scrollBy(int, int)), SLOT(updateWidgets()));
}

/**
 *
*/
AdvListViewItem::AdvListViewItem( AdvListViewItem * parent, QString a, QString b )
   : QListViewItem( parent, a, b )
{
  init();
}

/**
 *
*/
void AdvListViewItem::init()
{
  clearAllColumnWidgets();
}
		
/**
 *
*/
void AdvListViewItem::setColumnWidget( int col, AdvLvi_Base *wp, bool activated )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return;

  colwid[ col ] = wp;
  colactivated[ col ] = activated;
}

/**
 *
*/
void AdvListViewItem::clearColumnWidget( int col, bool deleteit )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return;

  if ( !getColumnWidget( col ) )
    return;

  if ( deleteit )
    delete getColumnWidget( col );

  setColumnWidget( col, 0 );
}

/**
 *
*/
void AdvListViewItem::clearAllColumnWidgets( bool deletethem )
{
  for (int i=0; i<=MAX_WIDGETCOLS_PER_LINE; i++)
    clearColumnWidget(i, deletethem);
}

/**
 *
*/
QWidget* AdvListViewItem::getColumnWidget( int col )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return 0;

  return colwid[ col ];
}

/**
 *
*/
void AdvListViewItem::activateColumnWidget( int col, bool activate )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return;

  colactivated[ col ] = activate;
}

/**
 *
*/
bool AdvListViewItem::columnWidgetActive( int col )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return false;

  return colactivated[ col ];
}


void AdvListViewItem::testAndResizeAllWidgets()
{
  for (int i = 0; i < MAX_WIDGETCOLS_PER_LINE; i++)
    if (colwid[i])
      testAndResizeWidget( i );
}



void AdvListViewItem::updateWidgets()
{
  testAndResizeAllWidgets();

  AdvListViewItem* i = (AdvListViewItem*)firstChild();

  while (i) {
    i->updateWidgets();
    i = (AdvListViewItem*)i->nextSibling();
  } ;
}					



void AdvListViewItem::testAndResizeWidget(int column)
{
  // if the given column is in  valid range for widgets and
  //   we have  been given  a valid pointer  on a widget we
  //   continue the widget handling.
  if ((column >= 0) && (column <= MAX_WIDGETCOLS_PER_LINE) && (colwid[ column ]))
    {

#if defined(LVDEBUG)
    debug("testAndResizeWidget %s, (selected=%d)", text(column), isSelected());
#endif

      // if the line the class  builds is selected and if the
      //   widget  for this column  is active we  can show it
      //   otherwise we have to hide and take  the focus away
      //   from it.
      if (isSelected() && (columnWidgetActive( column )) && ((!parent()) || (parent()->isOpen())))
        {
          colwid[column]->show();
          listView()->ensureItemVisible(this);
        }
      else
        {
          colwid[column]->hide();

          if (colwid[column])
            setText(column, colwid[column]->getText());

          colwid[column]->clearFocus();
        }

    }

}






/**
 * Overloaded QListViewItem::paintCell() method. This method tests
 * if we  have a widget  prepared for this  cell and if we have it
 * shows and moves or hides it.
*/		
void AdvListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width, int align )
{
  if ( !p )
    return;

  QListView *lv = listView();

  if (!lv)
    return;


  // if the given column is in  valid range for widgets and
  //   we have  been given  a valid pointer  on a widget we
  //   continue the widget handling. Otherwise we just call
  //   the parents paintCell() method (see below).
  if ((column >= 0) && (column <= MAX_WIDGETCOLS_PER_LINE) && (colwid[ column ]))
    {

#if defined(LVDEBUG)
    debug("paintCell %s, (selected=%d)", text(column), isSelected());
#endif

      // if the line the class  builds is selected and if the
      //   widget for this column is active we can move it to
      //   the right place  and show it  otherwise we have to
      //   hide and take the focus away from it.

      if ((lv->isSelected(this)) && (columnWidgetActive( column )))
        {
          QRect r(lv->itemRect( this ));

          // test if the column is _completely_ visible by testing
          //   wheter it is  not starting at y 0 or,  if it is the
          //   first item, whether it is not starting at y under 0
          if ( (r.y()>0) || ((lv->firstChild() == this) && (r.y()>=0) ) )
            {
              int x = 0;
              int dummy = column;
              while (dummy--)
                x += lv->columnWidth(dummy);

              x = (int)p->worldMatrix().dx();

              // we have to test if the colums goes out of the
              //   visible (x or width) area and if it does we
              //   have to resize it in order to avoid that it
              //   overpaintes the vertical scrollbar.
              int wd = width;
              int vs = 0;
              if (lv->verticalScrollBar()->isVisible())
                vs = lv->verticalScrollBar()->width();

              if (x+width > lv->width() - vs)
                wd = lv->width() - vs - x;

              colwid[column]->setGeometry( x,
                                    r.y() + lv->viewport()->y(),
                                    wd, height());

              colwid[column]->show();
            }
          else
            {
              colwid[column]->hide();

              if (colwid[column])
                setText(column, colwid[column]->getText());
            }


          // if more than one widget exists and one of these do have
          //   the focus we do not  need to set it to this (makes it
          //   possible to jump between  several widgets in one line
          //   using the tabulator [tab] key)
          bool flag = true;
          for (int i=0; i < MAX_WIDGETCOLS_PER_LINE; i++)
            if (colwid[i])
              if (colwid[i]->hasFocus())
                flag = false;

          // if we do not have found a widget with the focus in this
          //   line (see above) we must  set the focus to the one in
          //   this line and column.
          if (flag)
            colwid[column]->setFocus();
          else
            colwid[column]->clearFocus();
        }
      else
        {
          // since this  widget has been  disactivated or the line this
          //   instance of the class represents is not selected we have
          //   to hide the widget and take the focus away from it
          colwid[column]->hide();

          colwid[column]->clearFocus();
        }
    }


  QListViewItem::paintCell(p,cg,column, width,align);
}

KDlgPropWidget::KDlgPropWidget(CKDevelop* parCKD, QWidget *parent, const char *name ) : QWidget(parent,name)
{
  pCKDevel = parCKD;
  lv = new AdvListView(this);
  lv->addColumn(i18n("Property"));
  lv->addColumn(i18n("Value"));
  lv->show();

  lv->setRootIsDecorated(false);
}

void KDlgPropWidget::refillList(KDlgItem_Base* source)
{
  if (!source)
    return;

  lv->clear();

  QString grps[32];
  AdvListViewItem *grpLvis[32];
  int grpsCnt = 0;
  int numGrp;


  KDlgPropertyEntry *prop;
  for (int i = 0; i<=source->getProps()->getEntryCount(); i++)
    {
      prop = source->getProps()->getProp(i);

      numGrp = -1;
      for (int n = 0; n<grpsCnt; n++)
        if (prop->group.upper()==grps[n].upper())
          {
            numGrp = n;
            break;
          }

      if (numGrp == -1)
        {
          numGrp = grpsCnt++;
          grps[numGrp] = prop->group;
          if (grps[numGrp].length()!=0)
            grpLvis[numGrp] = new AdvListViewItem(lv,grps[numGrp],"");
          else
            continue;
        }

      AdvListViewItem *lvi = new AdvListViewItem(grpLvis[numGrp],prop->name,prop->value);
      switch (prop->allowed)
        {
          case ALLOWED_STRING:
            lvi->setColumnWidget(1, new AdvLvi_String( lv, prop ));
            break;
          case ALLOWED_BOOL:
            lvi->setColumnWidget(1, new AdvLvi_Bool( lv, prop ));
            break;
          case ALLOWED_INT:
            lvi->setColumnWidget(1, new AdvLvi_Int( lv, prop ));
            break;
          case ALLOWED_FILE:
            lvi->setColumnWidget(1, new AdvLvi_Filename( lv, prop ));
            break;
          case ALLOWED_COLOR:
            lvi->setColumnWidget(1, new AdvLvi_ColorEdit( lv, prop ));
            break;
          case ALLOWED_FONT:
            lvi->setColumnWidget(1, new AdvLvi_Font( lv, prop ));
            break;
          case ALLOWED_CONNECTIONS:
    //        lvi->setColumnWidget(1, new AdvLvi_Connections( lv, prop ));
            break;
          case ALLOWED_CURSOR:
            lvi->setColumnWidget(1, new AdvLvi_Cursor( lv, prop ));
            break;
          case ALLOWED_BGMODE:
            lvi->setColumnWidget(1, new AdvLvi_BgMode( lv, prop ));
            break;
       }

    }
}

KDlgPropWidget::~KDlgPropWidget()
{
}

void KDlgPropWidget::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent( e );

  lv->setGeometry(0,0,width(),height());
}










AdvLvi_Base::AdvLvi_Base(QWidget *parent, KDlgPropertyEntry *dpe, const char *name)
  : QWidget( parent, name )
{
  setGeometry(0,0,0,0);
  propEntry = dpe;
  setBackgroundColor( colorGroup().base() );
//  setFocusPolicy(NoFocus);
  setEnabled(false);
}

void AdvLvi_Base::paintEvent ( QPaintEvent * e )
{
  setBackgroundColor( colorGroup().base() );
  QWidget::paintEvent( e );
}



AdvLvi_String::AdvLvi_String(QWidget *parent, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, dpe, name )
{
  setGeometry(0,0,0,0);
  leInput = new QLineEdit( this );
  leInput->setText(dpe->value);
  connect(leInput, SIGNAL(textChanged ( const char * )), SLOT(returnPressed()));
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
}


AdvLvi_Int::AdvLvi_Int(QWidget *parent, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, dpe, name )
{
  setGeometry(0,0,0,0);

  leInput = new KRestrictedLine( this, 0, " 01234567890-" );
  leInput->setText(dpe->value.stripWhiteSpace());

  connect(leInput, SIGNAL(textChanged ( const char * )), SLOT(returnPressed()));
  connect(leInput, SIGNAL(returnPressed()), SLOT(returnPressed()));

  up = new QPushButton("",this);
  down = new QPushButton("",this);
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
}



AdvLvi_ExtEdit::AdvLvi_ExtEdit(QWidget *parent, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, dpe, name )
{
  setGeometry(0,0,0,0);
  btnMore = new QPushButton("...",this);
  leInput = new QLineEdit( this );
  leInput->setText(dpe->value);

  connect(leInput, SIGNAL(textChanged ( const char * )), SLOT(returnPressed()));
  connect(leInput, SIGNAL(returnPressed()), SLOT(returnPressed()));
//  connect( leInput, SIGNAL( textChanged ( const char * ) ), SLOT( updateParentLvi() ) );
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
}


AdvLvi_Filename::AdvLvi_Filename(QWidget *parent, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_ExtEdit( parent, dpe, name )
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


AdvLvi_Bool::AdvLvi_Bool(QWidget *parent, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dpe, name )
{
  setGeometry(0,0,0,0);
  cbBool = new QComboBox( FALSE, this );
  cbBool->insertItem(i18n("TRUE"));
  cbBool->insertItem(i18n("FALSE"));
  cbBool->setCurrentItem(isValueTrue(dpe->value) ? 0 : 1);
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
    return cbBool->currentItem() ? "FALSE" : "TRUE";
  else
    return QString();
}

void AdvLvi_Bool::activated( const char* s )
{
  propEntry->value = cbBool->currentItem() ? "FALSE" : "TRUE";
}

AdvLvi_ColorEdit::AdvLvi_ColorEdit(QWidget *parent, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dpe, name )
{
  setGeometry(0,0,0,0);
  btn = new KColorButton(this);
}

void AdvLvi_ColorEdit::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (btn)
    btn->setGeometry(0,0,width(),height()+1);
}

QString AdvLvi_ColorEdit::getText()
{
  if (!btn) return QString();
  char s[255];
  sprintf(s,"0x%.2x%.2x%.2x",btn->color().red(),btn->color().green(),btn->color().blue());
  return QString(s);
}

QString AdvLvi_intToHex(int i)
{
  char s[10];
  sprintf(s,"%.2x",i);
  return QString(s);
}

void AdvLvi_ColorEdit::changed ( const QColor &newColor )
{
//  int l=0;
//  sscanf((const char*)getText(), "%i", &l);
//  QColor myColor(QColor((unsigned char)(l>>16),(unsigned char)(l>>8),(unsigned char)(l)));
//  int res = KColorDialog::getColor( myColor );
  propEntry->value = "0x"+AdvLvi_intToHex(newColor.red())+AdvLvi_intToHex(newColor.green())+AdvLvi_intToHex(newColor.blue());
}


AdvLvi_Font::AdvLvi_Font(QWidget *parent, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_ExtEdit( parent, dpe, name )
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
    }
}



AdvLvi_BgMode::AdvLvi_BgMode(QWidget *parent, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dpe, name )
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
    return cbBool->currentItem() ? QString(cbBool->currentText()) : QString();
  else
    return QString();
}

void AdvLvi_BgMode::activated( const char* s )
{
  propEntry->value = getText();
}


AdvLvi_Cursor::AdvLvi_Cursor(QWidget *parent, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, dpe, name )
{
  setGeometry(0,0,0,0);
  cbBool = new QComboBox( FALSE, this );
  cbBool->insertItem(i18n("(not set)"));
  cbBool->insertItem("handCursor");
  cbBool->insertItem("arrowCursor");
  cbBool->insertItem("upArrowCursor");
  cbBool->insertItem("crossCursor");
  cbBool->insertItem("waitCursor");
  cbBool->insertItem("ibeamCursor");
  cbBool->insertItem("sizeVerCursor");
  cbBool->insertItem("sizeHorCursor");
  cbBool->insertItem("sizeBDiagCursor");
  cbBool->insertItem("sizeFDiagCursor");
  cbBool->insertItem("sizeAllCursor");
  cbBool->insertItem("blankCursor");
  cbBool->setCurrentItem(0);
}

void AdvLvi_Cursor::resizeEvent ( QResizeEvent *e )
{
  AdvLvi_Base::resizeEvent( e );

  if (cbBool)
    cbBool->setGeometry(0,0,width(),height()+1);
}

QString AdvLvi_Cursor::getText()
{
  if (cbBool)
    return cbBool->currentItem() ? QString(cbBool->currentText()) : QString();
  else
    return QString();
}


void AdvLvi_Cursor::activated( const char* s )
{
  propEntry->value = getText();
}


