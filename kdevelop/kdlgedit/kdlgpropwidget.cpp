/***************************************************************************
                             kdlgpropwidget.cpp
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


#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <stdio.h>
#include "kdlgedit.h"
#include "kdlgproplv.h"
#include "kdlgproplvis.h"
#include "kdlgpropwidget.h"
#include "kdlgeditwidget.h"
#include "kdlgitembase.h"
#include "kdlgpropertybase.h"
#include "defines.h"
#include "itemsglobal.h"


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


KDlgPropWidget::KDlgPropWidget(KDlgEdit *dlged, QWidget *parent, const char *name )
    : QWidget(parent,name)
{
    fillHelps();

    dlgedit = dlged;
    lv = new AdvListView(this, "kdlg property listview");
    lv->setRootIsDecorated(false);
    lv->addColumn(i18n("Property"));
    lv->addColumn(i18n("Value"));
    lv->show();
    
    connect(lv, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(helpRequested(QListViewItem*)));
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

  QPixmap folder_pix = KGlobal::iconLoader()->loadApplicationMiniIcon("folder.png");

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
            adv = new AdvLvi_String( lv, dlgedit, prop );
            break;
          case ALLOWED_BOOL:
            adv = new AdvLvi_Bool( lv, dlgedit, prop );
            val = val.lower();
            break;
          case ALLOWED_ORIENTATION:
            adv = new AdvLvi_Orientation( lv, dlgedit, prop );
            break;
          case ALLOWED_INT:
            adv = new AdvLvi_Int( lv, dlgedit, prop );
            break;
	case ALLOWED_UINT:
            adv = new AdvLvi_UInt( lv, dlgedit, prop );
            break;
          case ALLOWED_FILE:
            adv = new AdvLvi_Filename( lv, dlgedit, prop );
            break;
          case ALLOWED_COLOR:
            adv = new AdvLvi_ColorEdit( lv, dlgedit, prop );
            break;
          case ALLOWED_FONT:
            adv = new AdvLvi_Font( lv, dlgedit, prop );
            break;
          case ALLOWED_CONNECTIONS:
    //        adv = new AdvLvi_Connections( lv, dlgedit, prop );
            break;
          case ALLOWED_CURSOR:
            adv = new AdvLvi_Cursor( lv, dlgedit, prop );
            break;
          case ALLOWED_VARNAME:
            adv = new AdvLvi_Varname( lv, dlgedit, prop );
            break;
          case ALLOWED_COMBOLIST:
            adv = new AdvLvi_ComboList( lv, dlgedit, prop );
            break;
          case ALLOWED_MULTISTRING:
            adv = new AdvLvi_MultiString( lv, dlgedit, prop );
            val = "[QStrings...]";
            break;
        }

        AdvListViewItem *lvi = new AdvListViewItem(grpLvis[numGrp],prop->name,val);
        if (adv)
          lvi->setColumnWidget(adv);

    }

  lv->restoreOpenStats();
}

#include <iostream.h>
void KDlgPropWidget::helpRequested(QListViewItem *it)
{
  if (it)
    {
      KDlgItem_Base *bi = dlgedit->kdlg_get_edit_widget()->selectedWidget();
      QString st = i18n("Sorry, there is no help available for this property.");
      if (bi)
        {
          PropHelp *pph = &PropHelps[0];
          int a = 0;
          while (!pph->name.isNull())
            {
              if (pph->name.upper() == QString(it->text(0)).upper())
                {
                  if ((pph->type.isEmpty()) || (pph->type.upper() == bi->itemClass()))
                    {
                      st = pph->help;
                      break;
                    }
                }
              pph = &PropHelps[++a];
            }

        }

      st = st + i18n("\n\n<i><->See also: <link kdev>KDevelop Manual</link>, <link qt>Qt API</link>, <link kdeui>KDE User Interfaces</link>, <link KDEWEB>KDE Website</link><+></i>");

      QString helptext = KDlgLimitLines(st, 40);
      cout << "Help: " << helptext << endl;

#warning FIXME
#if 0
      KQuickHelpWindow *qhw = new KQuickHelpWindow();
      connect(qhw, SIGNAL(hyperlink(QString)), SLOT(linkclicked(QString)));
      qhw->popup(QString("<b><brown>")+it->text(0)+QString("</b><black>\n\n") + helptext, QCursor::pos().x(),QCursor::pos().y());
#endif
    }
}


void KDlgPropWidget::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent( e );

  lv->setGeometry(0,0,width(),height());
}
