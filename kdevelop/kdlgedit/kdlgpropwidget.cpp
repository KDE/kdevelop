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

#include "items.h"
#include "kdlgpropwidget.h"
#include "kdlgproplvis.h"
#include <kcolorbtn.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qpopupmenu.h>
#include <qmultilinedit.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <kfiledialog.h>
#include <krestrictedline.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <stdio.h>
#include "../ckdevelop.h"
#include "kdlgproplv.h"
#include "kdlgitems.h"
#include "items.h"
#include "itemsglobal.h"
#include "defines.h"


struct PropHelp
{
  QString type;
  QString name;
  QString help;
};

PropHelp PropHelps[100];

void setPropHelp(int nr, QString type, QString name, QString help)
{
  PropHelps[nr].type = type;
  PropHelps[nr].name = name;
  PropHelps[nr].help = help;
}

void fillHelps()
{
  int nr = 0;
  setPropHelp(nr++, "",                "General",         i18n("<i>Group</i>\n\nThis group contains general properties like <i>Name</i>or <i>Text</i>."));
  setPropHelp(nr++, "",                "Geometry",        i18n("<i>Group</i>\n\nThis group contains properties having to do with the placement of the item like <i>Width</i>or <i>Height</i>."));
  setPropHelp(nr++, "",                "Appearance",      i18n("<i>Group</i>\n\nThis group contains properties having to do with how the item will appear in the dialog.\n\n<i>Pixmap</i>is an example for such an appearance property."));
  setPropHelp(nr++, "",                "C++ Code",        i18n("<i>Group</i>\n\nThis group is a special one. It contains properties which do not effect on the item in the dialog editor but in the running program.\n\nThat means if you change them you will not see any difference in the editor but there can be a big difference in the running program.\n\n<i>Connections</i>might be a pretty good example."));

  setPropHelp(nr++, "",                "Name",            i18n("<i>Class : Any</i>\n\nHelps to identify an dialog item i.e. for runtime loaded dialogs. In addition, if the constructor of the destination class has a \"name\" argument it is set to this value."));
  setPropHelp(nr++, "",                "IsHidden",        i18n("<i>Class : Any</i>\n\nIf this property is set to <i>true</i>the item will not be visible in the editor and in the program.") );
  setPropHelp(nr++, "",                "IsEnabled",       i18n("<i>Class : Any</i>\n\nIf disabled (=<i>false</i>) the widget will not handle user events (i.e. mouse or keyboard events).") );

  setPropHelp(nr++, "",                "VarName",         i18n("<i>Class : Any</i>\n\nUsed for generating sources. Defines the variable later containing the items' pointer.") );
  setPropHelp(nr++, "",                "Connections",     i18n("<i>Class : Any</i>\n\nDefines the <i>SIGNAL/SLOT</i>connections for this item. (Only has effects in programs, not in this editor)") );
  setPropHelp(nr++, "",                "ResizeToParent",  i18n("<i>Class : Any</i>\n\nIs <i>true</i>this item will fill out the whole of his parent window. (Only has effects in programs, not in this editor)") );
  setPropHelp(nr++, "",                "AcceptsDrops",    i18n("<i>Class : Any</i>\n\nIf enabled (=<i>true</i>) the widget will accept drops from the drag&drop mechanism. (Only has effects in programs, not in this editor)") );
  setPropHelp(nr++, "",                "HasFocus",        i18n("<i>Class : Any</i>\n\nIf <i>true</i>and if it is the only widget whose \'HasFocus\' property is enabled it will have the focus by default.\n\n<b><u>Note</u></b>: Only one widget can have focus at the same time. So do <i>not</i>enable this property for more than one dialog item !") );
  setPropHelp(nr++, "",                "FocusProxy",      i18n("<i>Class : Any</i>\n\nIf you enter a widgets' variable name here it will be the focus proxy for this item (which means, if the widget you´ve entered gets the focus it transfers it to this widget)") );

  setPropHelp(nr++, "",                "X",               i18n("<i>Class : Any</i>\n\nThe X coordinate of the widget relative to its parent widget.") );
  setPropHelp(nr++, "",                "Y",               i18n("<i>Class : Any</i>\n\nThe Y coordinate of the widget relative to its parent widget.") );
  setPropHelp(nr++, "",                "Width",           i18n("<i>Class : Any</i>\n\nThe width of the widget.") );
  setPropHelp(nr++, "",                "Height",          i18n("<i>Class : Any</i>\n\nThe height of the widget.") );
  setPropHelp(nr++, "",                "MinWidth",        i18n("<i>Class : Any</i>\n\nDefines the minimum width of the widget. The widget than cannot become smaller than this value.") );
  setPropHelp(nr++, "",                "MinHeight",       i18n("<i>Class : Any</i>\n\nDefines the minimum height of the widget. The widget than cannot become smaller than this value.") );
  setPropHelp(nr++, "",                "MaxWidth",        i18n("<i>Class : Any</i>\n\nDefines the maximum width of the widget. The widget than cannot become larger than this value.") );
  setPropHelp(nr++, "",                "MaxHeight",       i18n("<i>Class : Any</i>\n\nDefines the maximum height of the widget. The widget than cannot become larger than this value.") );
  setPropHelp(nr++, "",                "IsFixedSize",     i18n("<i>Class : Any</i>\n\nIf <i>true</i>the widget size cannot be changed.") );
  setPropHelp(nr++, "",                "SizeIncX",        i18n("<i>Class : Any</i>\n\nSets in how many pixel steps the item should be resized.") );
  setPropHelp(nr++, "",                "SizeIncY",        i18n("<i>Class : Any</i>\n\nSets in how many pixel steps the item should be resized.") );

  setPropHelp(nr++, "",                "BgMode",          i18n("<i>Class : Any</i>\n\nThe background mode of the widget.") );
  setPropHelp(nr++, "",                "BgColor",         i18n("<i>Class : Any</i>\n\nThe background color of the widget. Overrides the BgMode property.") );
  setPropHelp(nr++, "",                "BgPalColor",      i18n("<i>Class : Any</i>\n\nThe (background) palette color. This more recent method for BgColor also changes the frames etc.") );
  setPropHelp(nr++, "",                "BgPixmap",        i18n("<i>Class : Any</i>\n\nSpecifies the file which should be used as background pixmap. Note that not all widgets run correctly with background images. (i.e. the QLineEdit)") );
  setPropHelp(nr++, "",                "MaskBitmap",      i18n("<i>Class : Any</i>\n\nSpecifies the file which should be used as mask bitmap. Note that this operation can be pretty slow.") );
  setPropHelp(nr++, "",                "Font",            i18n("<i>Class : Any</i>\n\nSets the font which should be used for drawing text in the widget.") );
  setPropHelp(nr++, "",                "Cursor",          i18n("<i>Class : Any</i>\n\nSets the cursor which should appear when moving the mouse pointer over the widget.") );

  setPropHelp(nr++, "QPushButton",     "Text",            i18n("<i>Class : QPushButton</i>\n\nThe text which will appear on the button.") );
  setPropHelp(nr++, "QPushButton",     "isDefault",       i18n("<i>Class : QPushButton</i>\n\nWhen enabled, the button will be the default button. That means if the user presses return this button will receive the clicked() signal. Note that only one button can be the default button and that this value is only valid in dialogs.") );
  setPropHelp(nr++, "QPushButton",     "isAutoDefault",   i18n("<i>Class : QPushButton</i>\n\nWhen enabled, the button will be an auto default button. An auto-default button becomes the default push button automatically when it receives the keyboard input focus.") );
  setPropHelp(nr++, "QPushButton",     "isToggleButton",  i18n("<i>Class : QPushButton</i>\n\nWhen enabled, the button keeps pressed until it is pressed again.") );
  setPropHelp(nr++, "QPushButton",     "isToggledOn",     i18n("<i>Class : QPushButton</i>\n\nWhen enabled, and isToggleButton is also enabled the button will be toggled down by default.") );
  setPropHelp(nr++, "QPushButton",     "isMenuButton",    i18n("<i>Class : QPushButton</i>\n\nTells this button to draw a menu indication triangle if enabled, and to not draw one if disabled.") );
  setPropHelp(nr++, "QPushButton",     "isAutoResize",    i18n("<i>Class : QPushButton</i>\n\nWhen enabled, the button will resize itself whenever the contents change.") );
  setPropHelp(nr++, "QPushButton",     "isAutoRepeat",    i18n("<i>Class : QPushButton</i>\n\nWhen enabled, the clicked() signal is emitted at regular intervals while the buttons is down. Note that this property does not have any effect if isToggleButton is enabled.") );
  setPropHelp(nr++, "QPushButton",     "Pixmap",          i18n("<i>Class : QPushButton</i>\n\nThe pixmap which will appear on the button. Leave it empty if you don't want a pixmap on the button.") );

  setPropHelp(nr++, "QLineEdit",       "Text",            i18n("<i>Class : QLineEdit</i>\n\nThis text is set to the lineedit after it is created.") );
  setPropHelp(nr++, "QLineEdit",       "hasFrame",        i18n("<i>Class : QLineEdit</i>\n\nSets the singleline editor to draw itself inside a two-pixel frame if enabled, otherwise to draw itself without any frame.") );
  setPropHelp(nr++, "QLineEdit",       "MaxLength",       i18n("<i>Class : QLineEdit</i>\n\nSets the maximum length of the text in the editor.") );
  setPropHelp(nr++, "QLineEdit",       "CursorPositon",   i18n("<i>Class : QLineEdit</i>\n\nSet the cursor position for this line edit.") );
  setPropHelp(nr++, "QLineEdit",       "isTextSelected",  i18n("<i>Class : QLineEdit</i>\n\nWhen enabled the text is initially selected.") );
  setPropHelp(nr++, "QGroupBox",       "Title",           i18n("<i>Class : QGroupBox</i>\n\nSets the groupbox title that is displayed in the box frame.") );
  setPropHelp(nr++, "QProgressBar",    "TotalSteps",      i18n("<i>Class : QProgressBar</i>\n\nSets the total steps of the progressbar. During the iteration of your action to display the progress, you have to call setProgress(int) to advance the progress step displayed to (int)") );
  setPropHelp(nr++, "QMultiLineEdit",  "setFixedVisibleLines",  i18n("<i>Class : QMultiLineEdit</i>\n\nSets a fixed height for the widget so that the given number of textlines are displayed using the current font.") );
  setPropHelp(nr++, "QMultiLineEdit",  "Text",            i18n("<i>Class : QMultiLineEdit</i>\n\nSets the text of the widget.") );
  setPropHelp(nr++, "QMultiLineEdit",  "isAutoUpdate",    i18n("<i>Class : QMultiLineEdit</i>\n\nUsed to avoid flicker during large changes; the view is not updated if disabled.") );
  setPropHelp(nr++, "QMultiLineEdit",  "isReadOnly",      i18n("<i>Class : QMultiLineEdit</i>\n\nSets the widget text to read only; disables text input.") );
  setPropHelp(nr++, "QMultiLineEdit",  "isOverWriteMode", i18n("<i>Class : QMultiLineEdit</i>\n\nSets overwrite enabled or disabled.") );
  setPropHelp(nr++, "QMultiLineEdit",  "isTextSelected",  i18n("<i>Class : QMultiLineEdit</i>\n\nMarks the whole text selected.") );
   setPropHelp(nr++, "QScrollBar",     "MaxValue",        i18n("<i>Class : QScrollBar</i>\n\nSets the maximum slider value; used in constructor") );
   setPropHelp(nr++, "QScrollBar",     "MinValue",        i18n("<i>Class : QScrollBar</i>\n\nSets the minimum slider value; used in constructor") );
   setPropHelp(nr++, "QScrollBar",     "Orientation",     i18n("<i>Class : QScrollBar</i>\n\nSets the orientation of the scrollbar to horizontal or vertical") );
   setPropHelp(nr++, "QScrollBar",     "Value",           i18n("<i>Class : QScrollBar</i>\n\nSets the initial value of the scrollbar in the constructor") );
   setPropHelp(nr++, "QScrollBar",     "isTracking",      i18n("<i>Class : QScrollBar</i>\n\nIf enabled, the scrollbar emits valueChanged() whenever the bar is dragged; otherwise only on mouse release.") );

   setPropHelp(nr++, "QSlider",        "MaxValue",        i18n("<i>Class : QSlider</i>\n\nsets the maximum slider value; used in constructor") );
   setPropHelp(nr++, "QSlider",        "MinValue",        i18n("<i>Class : QSlider</i>\n\nsets the minimum slider value; used in constructor") );
   setPropHelp(nr++, "QSlider",        "Orientation",     i18n("<i>Class : QSlider</i>\n\nsets the orientation of the slider to horizontal or vertical") );
   setPropHelp(nr++, "QSlider",        "Value",           i18n("<i>Class : QSlider</i>\n\nuses QRangeControl::setValue() to set the value") );
   setPropHelp(nr++, "QSlider",        "isTracking",      i18n("<i>Class : QSlider</i>\n\nif enabled, the slider emits valueChanged() whenever the bar is dragged; otherwise only on mouse release.") );
   setPropHelp(nr++, "QListBox",       "isAutoScroll",    i18n("<i>Class : QListBox</i>\n\nIf auto-scrolling is enabled, the list box will scroll its contents when the user drags the mouse outside (below or above) the list box.") );
   setPropHelp(nr++, "QListBox",       "isAutoBottomScrollBar",  i18n("<i>Class : QListBox</i>\n\nIf it is enabled, then the list box will get\na bottom scroll bar if the maximum list box item\nwidth exceeds the list box width.") );
   setPropHelp(nr++, "QListBox",       "isAutoScrollBar", i18n("<i>Class : QListBox</i>\n\nEnables an automatic (vertical) scroll bar.") );
   setPropHelp(nr++, "QListBox",       "isBottomScrollBar", i18n("<i>Class : QListBox</i>\n\nEnables a bottom scroll bar.") );
   setPropHelp(nr++, "QListBox",       "isDragSelect",    i18n("<i>Class : QListBox</i>\n\nIf drag-selection is enabled, the list box will highlight\nnew items when the user drags the mouse inside the list box. ") );
   setPropHelp(nr++, "QListBox",       "isSmoothScrolling", i18n("<i>Class : QListBox</i>\n\nEnables smooth list box scrolling.") );
   setPropHelp(nr++, "QListBox",       "setFixedVisibleLines",  i18n("<i>Class : QListBox</i>\n\nSets a fixed height for the widget so that the given number of textlines are displayed using the current font.") );
   setPropHelp(nr++, "QListBox",       "isAutoUpdate",    i18n("<i>Class : QListbox</i>\n\nUsed to avoid flicker during large changes; the view is not updated if disabled.") );

    
   
  setPropHelp(nr, QString(),QString(),QString());
  
}



bool isValueTrue(QString val)
{
  QString v(val.upper());

  if (v=="FALSE" || v=="0" || v=="NO" || v=="NULL" || v.isEmpty())
    return false;

  return true;
}


AdvListView::AdvListView( CKDevelop* parCKD, QWidget * parent , const char * name )
  : QListView( parent, name )
{
  pCKDevel = parCKD;
  int i;
  for (i=0; i<MAX_MAIN_ENTRYS; i++)
    openStats[i] = "";

  KConfig *config = kapp->getConfig();
  config->setGroup("KDlgEdit");
  QString stats = config->readEntry("PropOpenStats");

  if (stats.length()>0)
    {
      QString stat;
      int cnt = 0;
      while ( !(stat = getLineOutOfString(stats, cnt, ",")).isNull() )
        {
          openStats[cnt] = stat;
          cnt++;
        }


    }

}

AdvListView::~AdvListView()
{
  saveOpenStats();


  QString stats = "";

  int n;
  for (n=0; n<MAX_MAIN_ENTRYS; n++)
    {
      if (!openStats[n].isEmpty())
        stats = stats + openStats[n] + ",";
    }

  stats = stats.left(stats.length()-1);

  KConfig *config = kapp->getConfig();
  config->setGroup("KDlgEdit");
  config->writeEntry("PropOpenStats",stats);


  AdvListViewItem* i = (AdvListViewItem*)firstChild();

  while (i) {
    delete i;
    i = (AdvListViewItem*)i->nextSibling();
  }
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

  if (e->button() == RightButton)
    {
      QPopupMenu *phelp = new QPopupMenu;
      phelp->insertItem( i18n("&Help"), this, SLOT(help()) );
//      QPoint p(viewport()->mapFromGlobal (QCursor::pos()));
    //  QPoint p(QCursor::pos()); //mapFromGlobal (QCursor::pos()));
//      p.setY(p.y()-header()->height());
//      p.setX(0);
//      if (itemAt(p))
//        phelp->exec(QCursor::pos());
//      return;
help();
    }
  else
    {
      updateWidgets();
    }
}


void AdvListView::linkclicked(QString str)
{
  if (str.upper() == "KDEV")
    pCKDevel->slotHelpContents();
  if (str.upper() == "KDECORE")
    pCKDevel->slotHelpKDECoreLib();
  if (str.upper() == "KDEUI")
    pCKDevel->slotHelpKDEGUILib();
  if (str.upper() == "KDEKFILELIB")
    pCKDevel->slotHelpKDEKFileLib();
  if (str.upper() == "KDEHTML")
    pCKDevel->slotHelpKDEHTMLLib();
  if (str.upper() == "QT")
    pCKDevel->slotHelpQtLib();
  if (str.upper() == "C")
    pCKDevel->slotHelpReference();
  if (str.upper() == "ABOUT")
    pCKDevel->slotHelpAbout();
  if (str.upper() == "HOMEPAGE")
    pCKDevel->slotHelpHomepage();
  if (str.upper() == "MANUAL")
    pCKDevel->slotHelpManual();
  if (str.upper() == "KDEWEB")
    {
      // lets give this URL to kfm, he knows better what
      // to do with it
      if(vfork() > 0) {
	// drop setuid, setgid
	setgid(getgid());
	setuid(getuid());
	
	execlp("kfmclient", "kfmclient", "exec", QString("http://www.kde.org").data(), 0);
	_exit(0);
      }
    }

}

void AdvListView::help()
{
  QPoint p(viewport()->mapFromGlobal (QCursor::pos()));
//  p.setY(p.y()-header()->height());
//  p.setX(0);
  AdvListViewItem *it = (AdvListViewItem*)itemAt(p);
  if (it)
    {
      KDlgItem_Base *bi = pCKDevel->kdlg_get_edit_widget()->selectedWidget();
      QString st = i18n("Sorry, there is no help available for this property.");
      if (bi)
        {
          PropHelp *pph = &PropHelps[0];
          int a = 0;
          while (!pph->name.isNull())
            {
              if (pph->name.upper() == QString(it->text(0)).upper())
                {
                  if ((pph->type.isEmpty()) || (pph->type.upper() == bi->itemClass().upper()))
                    {
                      st = pph->help;
                      break;
                    }
                }
              pph = &PropHelps[++a];
            }

        }

      st = st + i18n("\n\n<i><->See also: <link kdev>KDevelop Manual</link>, <link qt>Qt API</link>, <link kdeui>KDE User Interfaces</link>, <link KDEWEB>KDE Website</link><+></i>");

      QString helptext = KDlgLimitLines(st);

#warning FIXME
#if 0
      KQuickHelpWindow *qhw = new KQuickHelpWindow();
      connect(qhw, SIGNAL(hyperlink(QString)), SLOT(linkclicked(QString)));
      qhw->popup(QString("<b><brown>")+it->text(0)+QString("</b><black>\n\n") + helptext, QCursor::pos().x(),QCursor::pos().y());
#endif
    }
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

void AdvListView::setGeometryEntrys(int x, int y, int w, int h)
{
  AdvListViewItem* i = (AdvListViewItem*)firstChild();

  while (i) {
    AdvListViewItem* n = (AdvListViewItem*)i->firstChild();
    while (n) {
      if (QString(n->text(0)).upper()=="X")
        n->setText(1, (const char*)(QString().setNum(x)));
      if (QString(n->text(0)).upper()=="Y")
        n->setText(1, (const char*)(QString().setNum(y)));
      if (QString(n->text(0)).upper()=="WIDTH")
        n->setText(1, (const char*)(QString().setNum(w)));
      if (QString(n->text(0)).upper()=="HEIGHT")
        n->setText(1, (const char*)(QString().setNum(h)));
      n = (AdvListViewItem*)n->nextSibling();
    }

    i = (AdvListViewItem*)i->nextSibling();
  }
}

void AdvListView::saveOpenStats()
{
  AdvListViewItem* it = (AdvListViewItem*)firstChild();

  while (it) {
    bool b = false;
    int i;
    for (i=0; i<MAX_MAIN_ENTRYS; i++)
      if (it->text(0) == openStats[i])
        {
          if (it->isOpen())
            {
              b = true;
              break;
            }
          else
            {
              openStats[i] = "";
            }
        }

    if ((!b) && (it->isOpen()))
      {
        for (i=0; i<MAX_MAIN_ENTRYS; i++)
          if (openStats[i] == "")
            {
              openStats[i] = it->text(0);
              break;
            }
      }

    it = (AdvListViewItem*)it->nextSibling();
  }
}

void AdvListView::restoreOpenStats()
{
  AdvListViewItem* it = (AdvListViewItem*)firstChild();

  while (it) {
    int i;
    for (i=0; i<MAX_MAIN_ENTRYS; i++)
      if (it->text(0) == openStats[i])
        it->setOpen(true);

    it = (AdvListViewItem*)it->nextSibling();
  }
}

void AdvListView::hideAll()
{
  AdvListViewItem* i = (AdvListViewItem*)firstChild();

  while (i) {
    i->hideWidgets();
    i = (AdvListViewItem*)i->nextSibling();
  }
}


AdvListViewItem::AdvListViewItem( QListView * parent, QString a, QString b)
   : QListViewItem( parent, a, b )
{
  init();
}

AdvListViewItem::AdvListViewItem( AdvListViewItem * parent, QString a, QString b )
   : QListViewItem( parent, a, b )
{
  init();
}

void AdvListViewItem::init()
{
  clearAllColumnWidgets();
}
		
void AdvListViewItem::hideWidgets()
{
  int i;
  for (i=0; i<MAX_WIDGETCOLS_PER_LINE; i++)
    if (colwid[i])
      colwid[i]->hide();

  AdvListViewItem* it = (AdvListViewItem*)firstChild();
  while (it) {
    it->hideWidgets();
    it = (AdvListViewItem*)it->nextSibling();
  }
}

void AdvListViewItem::setColumnWidget( int col, AdvLvi_Base *wp, bool activated )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return;

  colwid[ col ] = wp;
  colactivated[ col ] = activated;
}

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

void AdvListViewItem::clearAllColumnWidgets( bool deletethem )
{
  int i;
  for (i=0; i<MAX_WIDGETCOLS_PER_LINE; i++)
    clearColumnWidget(i, deletethem);
}

QWidget* AdvListViewItem::getColumnWidget( int col )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return 0;

  return colwid[ col ];
}

void AdvListViewItem::activateColumnWidget( int col, bool activate )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return;

  colactivated[ col ] = activate;
}

bool AdvListViewItem::columnWidgetActive( int col )
{
  if ( (col < 0) || (col > MAX_WIDGETCOLS_PER_LINE) )
    return false;

  return colactivated[ col ];
}


void AdvListViewItem::testAndResizeAllWidgets()
{
  int i;
  for (i = 0; i < MAX_WIDGETCOLS_PER_LINE; i++)
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
  if ((column >= 0) && (column < MAX_WIDGETCOLS_PER_LINE) && (colwid[ column ]))
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

          if (colwid[column]->getDisplayedText().isEmpty())
            setText(column, colwid[column]->getText());
          else
            setText(column, colwid[column]->getDisplayedText());

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
  if ((column >= 0) && (column < MAX_WIDGETCOLS_PER_LINE) && (colwid[ column ]))
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
                {
                  if (colwid[column]->getDisplayedText().isEmpty())
                    setText(column, colwid[column]->getText());
                  else
                    setText(column, colwid[column]->getDisplayedText());
                }
            }


          // if more than one widget exists and one of these do have
          //   the focus we do not  need to set it to this (makes it
          //   possible to jump between  several widgets in one line
          //   using the tabulator [tab] key)
          bool flag = true;
          int i;
          for (i=0; i < MAX_WIDGETCOLS_PER_LINE; i++)
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
  fillHelps();

  pCKDevel = parCKD;
  lv = new AdvListView(parCKD, this);
  lv->addColumn(i18n("Property"));
  lv->addColumn(i18n("Value"));
  lv->show();

  lv->setRootIsDecorated(false);
}

KDlgPropWidget::~KDlgPropWidget()
{
  delete lv;
}

void KDlgPropWidget::refillList(KDlgItem_Base* source)
{
  if (!source)
    return;

  lv->hideAll();
  lv->saveOpenStats();
  lv->clear();

  QString grps[32];
  AdvListViewItem *grpLvis[32];
  int grpsCnt = 0;
  int numGrp;
  AdvLvi_Base *adv = 0;

  KIconLoader *icon_loader = KGlobal::iconLoader();

  QPixmap folder_pix = icon_loader->loadIcon("folder.xpm");

  KDlgPropertyEntry *prop;
  int i,n;
  for (i = 0; i<=source->getProps()->getEntryCount(); i++)
    {
      prop = source->getProps()->getProp(i);

      numGrp = -1;
      for (n = 0; n<grpsCnt; n++)
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
            {
              grpLvis[numGrp] = new AdvListViewItem(lv,grps[numGrp],"");
              grpLvis[numGrp]->setPixmap(0,folder_pix);
            }
          else
            continue;
        }

      adv = 0;
      QString val = prop->value;
      switch (prop->allowed)
        {
          case ALLOWED_STRING:
            adv = new AdvLvi_String( lv, pCKDevel, prop );
            break;
          case ALLOWED_BOOL:
            adv = new AdvLvi_Bool( lv, pCKDevel, prop );
            val = val.lower();
            break;
          case ALLOWED_ORIENTATION:
            adv = new AdvLvi_Orientation( lv, pCKDevel, prop );
            break;
          case ALLOWED_INT:
            adv = new AdvLvi_Int( lv, pCKDevel, prop );
            break;
	case ALLOWED_UINT:
            adv = new AdvLvi_UInt( lv, pCKDevel, prop );
            break;
          case ALLOWED_FILE:
            adv = new AdvLvi_Filename( lv, pCKDevel, prop );
            break;
          case ALLOWED_COLOR:
            adv = new AdvLvi_ColorEdit( lv, pCKDevel, prop );
            break;
          case ALLOWED_FONT:
            adv = new AdvLvi_Font( lv, pCKDevel, prop );
            break;
          case ALLOWED_CONNECTIONS:
    //        adv = new AdvLvi_Connections( lv, pCKDevel, prop );
            break;
          case ALLOWED_CURSOR:
            adv = new AdvLvi_Cursor( lv, pCKDevel, prop );
            break;
          case ALLOWED_VARNAME:
            adv = new AdvLvi_Varname( lv, pCKDevel, prop );
            break;
          case ALLOWED_COMBOLIST:
            adv = new AdvLvi_ComboList( lv, pCKDevel, prop );
            break;
          case ALLOWED_MULTISTRING:
            adv = new AdvLvi_MultiString( lv, pCKDevel, prop );
            val = "[QStrings...]";
            break;
        }

/*        QString val = prop->value;

        if (adv)
          {
            val = adv->getText();
            if (val.isNull())
              val = "";
          }


        if (prop->allowed == ALLOWED_MULTISTRING)
          val = "[QStrings...]";

        AdvListViewItem *lvi = new AdvListViewItem(grpLvis[numGrp],prop->name,val);
        if (adv)
          lvi->setColumnWidget(1, adv);*/

        AdvListViewItem *lvi = new AdvListViewItem(grpLvis[numGrp],prop->name,val);
        if (adv)
          lvi->setColumnWidget(1, adv);

    }

  lv->restoreOpenStats();
}

void KDlgPropWidget::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent( e );

  lv->setGeometry(0,0,width(),height());
}










AdvLvi_Base::AdvLvi_Base(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : QWidget( parent, name )
{
  pCKDevel = parCKD;
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

void AdvLvi_Base::refreshItem()
{
  KDlgItem_Base *selit = pCKDevel->kdlg_get_edit_widget()->selectedWidget();
  if (selit)
    ((KDlgItem_Widget*)selit)->repaintItem((QFrame*)selit->getItem());

  if (propEntry->name.upper() == "NAME")
    pCKDevel->kdlg_get_items_view()->refreshList();

  pCKDevel->kdlg_get_edit_widget()->setModified(true);
}

AdvLvi_String::AdvLvi_String(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, parCKD, dpe, name )
{
  setGeometry(0,0,0,0);
  leInput = new QLineEdit( this );
  leInput->setText(dpe->value);
  connect(leInput, SIGNAL(textChanged (const QString &)), SLOT(returnPressed()));
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


AdvLvi_Int::AdvLvi_Int(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, parCKD, dpe, name )
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

AdvLvi_UInt::AdvLvi_UInt(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, parCKD, dpe, name )
{
  setGeometry(0,0,0,0);

  leInput = new KRestrictedLine( this, 0, " 01234567890" );
  leInput->setText(dpe->value.stripWhiteSpace());

  connect(leInput, SIGNAL(textChanged ( const char * )), SLOT(returnPressed()));
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



AdvLvi_ExtEdit::AdvLvi_ExtEdit(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, parCKD, dpe, name )
{
  setGeometry(0,0,0,0);
  btnMore = new QPushButton("...",this);
  leInput = new QLineEdit( this );
  leInput->setText(dpe->value);
  connect(leInput, SIGNAL(textChanged (const QString &)), SLOT(returnPressed()));
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
  refreshItem();
}


AdvLvi_Filename::AdvLvi_Filename(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_ExtEdit( parent, parCKD, dpe, name )
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



AdvLvi_Bool::AdvLvi_Bool(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, parCKD, dpe, name )
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
//  cbBool = new QCheckBox(isValueTrue(dpe->value) ? QString(i18n("true")) : QString(i18n("false")), this);
//  cbBool->setChecked(isValueTrue(dpe->value));
//  cbBool->setBackgroundMode( PaletteLight );
//  connect(cbBool, SIGNAL(clicked()), SLOT(btnPressed()));
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

//void AdvLvi_Bool::btnPressed()
void AdvLvi_Bool::activated( const QString &s )
{
  propEntry->value = getText();
//  cbBool->setText(cbBool->isChecked() ? "true" : "false");
//  propEntry->value = cbBool->text();
  refreshItem();
}


AdvLvi_Orientation::AdvLvi_Orientation(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, parCKD, dpe, name )
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






AdvLvi_ColorEdit::AdvLvi_ColorEdit(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, parCKD, dpe, name )
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


AdvLvi_Font::AdvLvi_Font(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_ExtEdit( parent, parCKD, dpe, name )
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



AdvLvi_BgMode::AdvLvi_BgMode(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, parCKD, dpe, name )
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


AdvLvi_Cursor::AdvLvi_Cursor(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, parCKD, dpe, name )
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


AdvLvi_Varname::AdvLvi_Varname(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_String(parent, parCKD, dpe, name)
{
  connect(leInput, SIGNAL(textChanged(const QString &)), this, SLOT(VarnameChanged()) );
}

void AdvLvi_Varname::VarnameChanged()
{
  pCKDevel->kdlg_get_edit_widget()->setVarnameChanged(true);
}

AdvLvi_ComboList::AdvLvi_ComboList(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name)
  : AdvLvi_Base( parent, parCKD, dpe, name )
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




AdvLvi_MultiString::AdvLvi_MultiString(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name )
  : AdvLvi_Base( parent, parCKD, dpe, name )
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
