/*****************************************************************************
                          kdlgeditwidget.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by                          
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it usnder the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "kdlgeditwidget.h"
#include "kdlgproplvis.h"
#include "../ckdevelop.h"
#include <qmessagebox.h>
#include <kapp.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <kruler.h>
#include <stdio.h>
#include "items.h"
#include "kdlgproplv.h"
#include "kdlgitems.h"
#include "kdlgpropwidget.h"
#include <kquickhelp.h>

KDlgEditWidget::KDlgEditWidget(CKDevelop* parCKD,QWidget *parent, const char *name )
   : QWidget(parent,name)
{
  qhw = new KQuickHelpWindow();
  dlgfilelinecnt = 0;
  pCKDevel = parCKD;

  setBackgroundMode(PaletteLight);

  rulh = new KRuler(KRuler::horizontal, this);
  rulh->setRulerStyle(KRuler::pixel);
  rulh->setRange(0,400);
  rulh->setOffset(0);
  rulh->setPixelPerMark(5);
  rulh->setEndLabel("");

  rulh->setFrameStyle(0);

  rulv = new KRuler(KRuler::vertical, this);
  rulv->setRulerStyle(KRuler::pixel);
  rulv->setRange(0,300);
  rulv->setOffset(0);
  rulv->setPixelPerMark(5);
  rulv->setEndLabel("");

  rulv->setFrameStyle(rulh->frameStyle());

  setGridSize(10,10);

  QFrame *edgeFrame = new QFrame(this);
  edgeFrame->setGeometry(0,0,RULER_WIDTH, RULER_HEIGHT);
  edgeFrame->setFrameStyle(rulh->frameStyle());

  main_widget = new KDlgItem_Widget( this, this, true );
  selected_widget = 0;
  selectWidget(main_widget);

  main_widget->getProps()->setProp_Value("X","0");
  main_widget->getProps()->setProp_Value("Y","0");
  main_widget->getProps()->setProp_Value("Width","400");
  main_widget->getProps()->setProp_Value("Height","300");
//  main_widget->addChild( new KDlgItem_PushButton ( this, main_widget->getItem() ) );
  main_widget->repaintItem();

  if ((parCKD) && parCKD->kdlg_get_items_view())
    parCKD->kdlg_get_items_view()->addWidgetChilds(main_widget);

  if ((parCKD) && parCKD->kdlg_get_prop_widget())
    parCKD->kdlg_get_prop_widget()->refillList(selected_widget);

  setModified(false);
  setWidgetAdded(false);
  setWidgetRemoved(false);
  setVarnameChanged(false);
}

KDlgEditWidget::~KDlgEditWidget()
{
}


void KDlgEditWidget::setGridSize(int x, int y)
{
  grid_size_x = x;
  grid_size_y = y;
  rulh->setPixelPerMark(x);
  rulv->setPixelPerMark(y);

  setModified(true);
}


int KDlgEditWidget::raiseSelected(bool updateMe)
{
  if (!selectedWidget())
    return -3;

  if (selectedWidget()==mainWidget())
    return -4;

  int res = 0;

  if (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem)
    {
      if (!((KDlgItem_Widget*)selectedWidget())->parentWidgetItem->getChildDb())
        return -4;
      res = ((KDlgItem_Widget*)selectedWidget())->parentWidgetItem->getChildDb()->raiseItem(selectedWidget());
    }
  else
    {
      res = mainWidget()->getChildDb()->raiseItem(selectedWidget());
    }

  if ((res == 0) && (updateMe))
    mainWidget()->recreateItem();

  setModified(true);

  return res;
}

int KDlgEditWidget::lowerSelected(bool updateMe)
{
  if (!selectedWidget())
    return -3;

  if (selectedWidget()==mainWidget())
    return -4;

  int res = 0;

  if (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem)
    {
      if (!((KDlgItem_Widget*)selectedWidget())->parentWidgetItem->getChildDb())
        return -4;
      res = ((KDlgItem_Widget*)selectedWidget())->parentWidgetItem->getChildDb()->lowerItem(selectedWidget());
    }
  else
    {
      res = mainWidget()->getChildDb()->lowerItem(selectedWidget());
    }

  if ((res == 0) && (updateMe))
    mainWidget()->recreateItem();

  setModified(true);

  return res;
}

void KDlgEditWidget::slot_raiseTopSelected()
{
  while (raiseSelected(false) == 0);

  mainWidget()->recreateItem();
}

void KDlgEditWidget::slot_lowerBottomSelected()
{
  while (lowerSelected(false) == 0);

  mainWidget()->recreateItem();
}

void KDlgEditWidget::slot_cutSelected()
{
  if ((!selected_widget) || (selected_widget == main_widget))
    return;

  slot_copySelected();
  slot_deleteSelected();
}

void KDlgEditWidget::slot_deleteSelected()
{
  if ((!selected_widget) || (selected_widget == main_widget))
    return;

  KDlgItem_Widget *dummy = (KDlgItem_Widget*)selected_widget;
  deselectWidget();
  selected_widget = 0;

  if (dummy->parentWidgetItem)
    {
      if (!dummy->parentWidgetItem->getChildDb())
        return;
      dummy->parentWidgetItem->getChildDb()->removeItem(dummy);
    }
  else
    {
      mainWidget()->getChildDb()->removeItem(dummy);
    }

  dummy->deleteMyself();
  delete dummy->getItem();
  delete dummy;
  selected_widget = 0;
  selectWidget(mainWidget());

  if ((pCKDevel) && ((CKDevelop*)pCKDevel)->kdlg_get_items_view())
    ((CKDevelop*)pCKDevel)->kdlg_get_items_view()->addWidgetChilds(main_widget);

  setModified(true);
  setWidgetRemoved(true);
}

void KDlgEditWidget::slot_copySelected()
{
  if (!selected_widget)
    return;

  QFile f("/tmp/kdevdlgedt_copyitem.tmp");
  if ( f.open(IO_WriteOnly) )
    {
      QTextStream t( &f );

      saveWidget((KDlgItem_Widget*)selected_widget, &t);
      f.close();
    }
  else
    {
      printf("kdlgedit ERROR : Could not open temporary file for copying action !\n");
    }
}

void KDlgEditWidget::slot_pasteSelected()
{
  KDlgItem_Widget *parent_widget = main_widget;

  if (selectedWidget())
    if (((selectedWidget()->itemClass().upper()=="QWIDGET") && (selectedWidget() != main_widget)) || (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem))
      {
        int res = 0;
        res = QMessageBox::information( this, i18n("Add item"),
                     i18n("Into which widget do you want to insert this item ?\n\n"
                     "You either may add it to the main widget or to the selected\n"
                     "widget respectively to the selected items' parent widget."),
                     i18n("&Main"), i18n("&Selected"), i18n("&Cancel"),0,2 );

        if (res == 1)
          {
            if ((selectedWidget()->itemClass().upper()=="QWIDGET") && (selectedWidget() != main_widget))
                parent_widget = (KDlgItem_Widget*)selectedWidget();
            else if (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem)
                parent_widget = ((KDlgItem_Widget*)selectedWidget())->parentWidgetItem;
          }
        else if (res == 2)
          return;
      }

  dlgfilelinecnt = 0;
  QFile f("/tmp/kdevdlgedt_copyitem.tmp");
  if ( f.open(IO_ReadOnly) )
    {
      QTextStream t( &f );

      while (!t.eof())
        {
          QString s;
          do s = dlgReadLine(&t); while (s.isEmpty());

          if (!readGrp_Item( parent_widget, &t, s.right(s.length()-s.find(' ')-1) ))
            {
              f.close();
              return;
            }

          KDlgItem_Base *it = parent_widget->getChildDb()->getFirst();
          KDlgItem_Base *lst = it;
          while (it)
            {
              lst = it;
              it = parent_widget->getChildDb()->getNext();
            }
          if (lst)
            {
              lst->getProps()->setProp_Value("X","10");
              lst->getProps()->setProp_Value("Y","10");
              lst->getItem()->move(10,10);
            }
        }

      f.close();
    }
  else
    {
      return;
    }

  getCKDevel()->kdlg_get_items_view()->refreshList();

  setWidgetAdded(true);
  setModified(true);
}

void KDlgEditWidget::slot_helpSelected()
{
  QString helptext = i18n("<b><u><brown>QuickHelp<black></u></b>\n\n");

  if (selectedWidget() != mainWidget())
    {
      helptext += i18n("<b><i>Raise</i></b>\n\nBrings the item one step towards the topmost one in the hierarchy.\n\n");
      helptext += i18n("<b><i>Lower</i></b>\n\nOn the contrary to <i>raise</i> this function brings the item one step backwards in the hierarchy, to the bottommost item.\n\n");
      helptext += i18n("<b><i>Raise to top</i></b>\n\nSets the item to be the topmost one.\n\n");
      helptext += i18n("<b><i>Raise to bottom</i></b>\n\nMakes the item to become the bottommost.\n\n");
      helptext += i18n("<b><i>Copy</i></b>\n\nSelects the item to be copied. That means, if you choose paste later, this item will be inserted.\n\n");
      helptext += i18n("<b><i>Delete</i></b>\n\nRemoves the selected item from the dialog.\n\n");
      helptext += i18n("<b><i>Cut</i></b>\n\nCopies the item and removes it from the dialog. But unlike toe delete function you will be able to insert a copy of the item using the <i>paste</i> function.\n\n");
    }
  else
    helptext += i18n("<b>Note :</b> Since the selected item is the main widget only few of the normally available functions are enabled. Open another item's help to get information about the other functions.\n\n");

  helptext += i18n("<b><i>Paste</i></b>\n\nInserts an item you have copied before using the <i>Copy</i>function at the current position of the mouse pointer.\n\n");

  helptext = KDlgLimitLines(helptext,60);

  qhw->popup(helptext, QCursor::pos().x(),QCursor::pos().y());
}


QString KDlgEditWidget::dlgReadLine( QTextStream *t )
{
  QString s;
  do {
    s = t->readLine().stripWhiteSpace();
    dlgfilelinecnt++;
  } while ( (!t->eof()) && ((s.left(2) == "//") || (s.isEmpty())) );

  if (s.left(2) == "//")
    return QString();

  if (s.isEmpty())
    return QString();

  return s;
}

bool KDlgEditWidget::readGrp_Ignore( QTextStream *t )
{
  QString s;
  int cnt = 0;
  do {
    s = dlgReadLine(t);
    if (s=="{") cnt++;
    if (s=="}")
      {
        if (cnt <= 1)
          return true;
        else
          cnt --;
      }
  } while (!t->eof());

  return true;
}

bool KDlgEditWidget::readGrp_Information( QTextStream *t )
{
  return readGrp_Ignore( t );
}

bool KDlgEditWidget::readGrp_SessionManagement( QTextStream *t )
{
  return readGrp_Ignore( t );
}

bool KDlgEditWidget::readGrp_Item( KDlgItem_Widget* par, QTextStream *t, QString ctype )
{
  int svdlc = dlgfilelinecnt;

  if (dlgReadLine(t) != "{")
    return false;

  KDlgItem_Widget *thatsme = 0;

  if (par)
    {
      thatsme = addItem(par, ctype);
    }
  else
    {
      deselectWidget();
      thatsme = mainWidget();
      thatsme->deleteMyself();
    }

  if (thatsme)
    {
      int i;
      for (i=1; i<=thatsme->getProps()->getEntryCount(); i++)
        thatsme->getProps()->setProp_Value(i,"");
      thatsme->getProps()->setProp_Value("VarName", typeCounter.countString(ctype));
    }
  else
    {
      int res = 0;
      QString errmsg = QString().sprintf(i18n("Error reading dialog file (line %d) :\n   Could not insert item (%s). Seems like I don't know it :-("), svdlc, (const char*)ctype);
      printf("  kdlgedit ERROR : %s\n", (const char*)errmsg);
      res = QMessageBox::warning( this, "Error", errmsg, i18n("&Ignore"), i18n("&Cancel"),0,1 );
      if (res == 1)
        {
          dlgfilelinecnt = -100;
          return false;
        }
    }

  QString s;
  do {
    s = dlgReadLine(t);
    if ((s.left(s.find(' ')).upper() == "ITEM") && (!thatsme))
      {
        printf("  kdlgedit : ignoring item \"%s\" line %d\n", (const char*)s.right(s.length()-s.find(' ')-1).upper(), dlgfilelinecnt);
        readGrp_Ignore( t );
      }
    if ((s!="}") && (thatsme))
      {
        if (s.left(s.find(' ')).upper() == "ITEM")
          {
            if ((ctype.upper() == "QWIDGET") || (ctype.upper() == "QFRAME"))
              {
                if (!readGrp_Item( thatsme, t, s.right(s.length()-s.find(' ')-1) ))
                  return false;
              }
            else
              {
                printf("  kdlgedit ERROR : line %d : \"%s\" cannot have child items.\n", dlgfilelinecnt, (const char*)ctype);
                readGrp_Ignore( t );
                return false;
              }
          }
        else
          {
            QString name  = s.left(s.find('='));
            QString value = s.right(s.length()-s.find('=')-2);
            value = value.left(value.length()-1);
            thatsme->getProps()->setProp_Value(name, value);
          }
      }
  } while ((!t->eof()) && (s!="}"));

  if (thatsme)
    thatsme->repaintItem();

  return true;
}


bool KDlgEditWidget::readGroup( QTextStream *t )
{
  QString s;
  s = dlgReadLine(t);
  if (s.isEmpty())
    return false;

  QString type = s.left(s.find(' ')).upper();
  QString name = s.right(s.length()-s.find(' ')-1);

  if ((s == "{") || (s == "}"))
    return true;

  printf("  kdlgedit : reading root %s-section \"%s\"\n",(const char*)type.lower(),(const char*)name);

  if (type == "DATA")
    {
      if (name.upper() == "INFORMATION")
        {
          if (!readGrp_Information( t ))
            return false;
        }
      else if (name.upper() == "SESSIONMANAGEMENT")
        {
          if (!readGrp_SessionManagement( t ))
            return false;
        }
      else
        {
          printf("  kdlgedit WARNING : line %d : unknown data type \"%s\". Section ignored.", dlgfilelinecnt, (const char*)name);
          if (!readGrp_Ignore( t ))
            return false;
        }
    }
  else if (type == "ITEM")
    {
      if (!readGrp_Item( 0, t, name ))
        return false;
    }

  return true;
}

bool KDlgEditWidget::openFromFile( QString fname )
{
  printf("kdlgedit : Loading dialog file \"%s\".\n", (const char*)fname);
  dlgfilelinecnt = 0;
  QFile f(fname);
  if ( f.open(IO_ReadOnly) )
    {
      QTextStream t( &f );

      typeCounter.clear();

      while (!t.eof())
        {
          if (!readGroup( &t ))
            {
              int res = 0;
              if (dlgfilelinecnt >=0)
                {
                  res = QMessageBox::warning( this, fname, i18n("Error while reading the dialog file.\n\n"
                               "Note : If you choose continue now the\ndialog may be loaded and saved incorrectly."),
                               i18n("&Ignore"), i18n("&Cancel"),0,1 );
                }
              else
                res = 1;
              if (res == 1)
                {
                  printf("  kdlgedit ERROR : %s \"%s\"\n", i18n("Aborted reading dialog file"), (const char*)fname);
                  QMessageBox::information( this, fname,
                           i18n("Reading aborted !\n\nThe dialog has not been loaded completely."
                                "\nOpen another dialog or create a new one\nif you don't like the result."));
                  f.close();
                  return false;
                }
            }
        }

      f.close();
    }
  else
    {
      printf("kdlgedit ERROR : Could not open dialog file !\n");
      return false;
    }

  printf("kdlgedit : Loading complete.\n");

  getCKDevel()->kdlg_get_items_view()->refreshList();
  selectWidget(mainWidget());

  setModified(false);
  setWidgetAdded(false);
  setWidgetRemoved(false);
  setVarnameChanged(false);

  return true;
}

bool KDlgEditWidget::saveToFile( QString fname )
{
  printf("kdlgedit : Writing dialog file \"%s\".\n", (const char*)fname);

  QFile f(fname);
  if ( f.open(IO_WriteOnly) )
    {
      QTextStream t( &f );

      printf("  kdlgedit : writing data sections\n");

      t << "// " << "KDevelop Dialog Editor File (.kdevdlg)\n";
      t << "// " << "\n";
      t << "// " << "Created by KDlgEdit Version " << KDLGEDIT_VERSION_STR << " (C) 1999 by Pascal Krahmer\n";
      t << "// " << "Get KDevelop including KDlgEdit at \"www.beast.de/kdevelop\"\n";
      t << "// " << "\n";
      t << "data Information\n";
      t << "{\n";
      t << "   Filename=\"" << fname << "\"\n";
      t << "   KDevelopVersion=\"" << KDEVELOP_VERSION_STR << "\"\n";
      t << "   DlgEditVersion=\"" << KDLGEDIT_VERSION_STR << "\"\n";
      t << "   LastChanged=\"" << QDateTime(QDate().currentDate(), QTime().currentTime()).toString() << "\"\n";
      t << "}\n";

      t << "\ndata SessionManagement\n";
      t << "{\n";

      AdvListView *lv = pCKDevel->kdlg_get_prop_widget()->getListView();
      lv->saveOpenStats();
      int i;
      int n=0;
      for (i=0; i<MAX_MAIN_ENTRYS; i++)
        {
          if (lv->openStat(i).length() != 0)
            t << "   " << "OpenedRoot_" << ++n << "=\"" << lv->openStat(i) << "\"\n";
        }

      t << "   " << "OpenedRootCount=\"" << n << "\"\n";
      t << "}\n";

      printf("  kdlgedit : writing item sections\n");
      saveWidget(mainWidget(), &t);
      f.close();

      setModified(false);
      setWidgetAdded(false);
      setWidgetRemoved(false);
    }
  else
    {
      printf("kdlgedit ERROR : Could not open dialog file !\n");
      return false;
    }

  printf("kdlgedit : Saving complete.\n");


  return true;
}

void KDlgEditWidget::saveWidget( KDlgItem_Widget *wid, QTextStream *t, int deep)
{
  if ((!wid) || (!t))
    return;

  int i;
  QString sDeep = "";

  for (i = 0; i<deep; i++)
    sDeep = sDeep + "   ";

  *t << "\n";
  *t << sDeep << "item " << wid->itemClass() << "\n";
  *t << sDeep << "{\n";

  for (i=1; i<=wid->getProps()->getEntryCount(); i++)
    {
      if (wid->getProps()->getProp(i)->value.length() > 0)
        *t << sDeep << "   " << wid->getProps()->getProp(i)->name << "=\"" << wid->getProps()->getProp(i)->value << "\"\n";
    }

  if (wid->itemClass().upper() == "QWIDGET")
    {
      KDlgItemDatabase *cdb = wid->getChildDb();
      if (cdb)
        {
          KDlgItem_Base *cdit = cdb->getFirst();
          while (cdit)
            {
              saveWidget( (KDlgItem_Widget*)cdit, t, deep+1 );
              cdit = cdb->getNext();
            }
        }
    }

  *t << sDeep << "}\n";
}

void KDlgEditWidget::newDialog()
{
  deselectWidget();
  mainWidget()->deleteMyself();

  getCKDevel()->kdlg_get_items_view()->refreshList();
  selectWidget(mainWidget());

  setModified(false);
  setWidgetAdded(false);
  setWidgetRemoved(false);
  setVarnameChanged(false);
}


void KDlgEditWidget::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent(e);

  rulh->setFixedHeight(RULER_WIDTH);
  rulv->setFixedWidth(RULER_HEIGHT);
  rulh->setGeometry(RULER_WIDTH,0,  width()-RULER_WIDTH, RULER_HEIGHT);
  rulv->setGeometry(0,RULER_HEIGHT, RULER_WIDTH, height()-RULER_HEIGHT);
}


void KDlgEditWidget::deselectWidget()
{
  if (selected_widget)
    {
      selected_widget -> deselect();
      selected_widget = 0;

      pCKDevel->disableCommand(ID_KDLG_EDIT_CUT);
      pCKDevel->disableCommand(ID_KDLG_EDIT_COPY);
      pCKDevel->disableCommand(ID_KDLG_EDIT_PASTE);
      pCKDevel->disableCommand(ID_KDLG_EDIT_DELETE);
    }
}

void KDlgEditWidget::selectWidget(KDlgItem_Base *i)
{
  if (i == selected_widget)
    return;

  KDlgItem_Base *wasSel = selected_widget;

  deselectWidget();

  selected_widget = i;

  if (selected_widget)
    {
      selected_widget -> select();
      if ((pCKDevel) && pCKDevel->kdlg_get_prop_widget())
         pCKDevel->kdlg_get_prop_widget()->refillList(selected_widget);
    }

  if (!wasSel)
    return;

  if (selected_widget != main_widget)
    {
      pCKDevel->enableCommand(ID_KDLG_EDIT_CUT);
      pCKDevel->enableCommand(ID_KDLG_EDIT_COPY);
      pCKDevel->enableCommand(ID_KDLG_EDIT_DELETE);
    }

  pCKDevel->enableCommand(ID_KDLG_EDIT_PASTE);
}


bool KDlgEditWidget::addItem(QString Name)
{
  KDlgItem_Base *par = main_widget;

  if (selectedWidget())
    if (((selectedWidget()->itemClass().upper()=="QWIDGET") && (selectedWidget() != main_widget)) || (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem))
      {
        int res = 0;
        res = QMessageBox::information( this, i18n("Add item"),
                     i18n("Into which widget do you want to insert this item ?\n\n"
                     "You either may add it to the main widget or to the selected\n"
                     "widget respectively to the selected items' parent widget."),
                     i18n("&Main"), i18n("&Selected"), i18n("&Cancel"),0,2 );

        if (res == 1)
          {
            if ((selectedWidget()->itemClass().upper()=="QWIDGET") && (selectedWidget() != main_widget))
                par = selectedWidget();
            else if (((KDlgItem_Widget*)selectedWidget())->parentWidgetItem)
                par = ((KDlgItem_Widget*)selectedWidget())->parentWidgetItem;
          }
        else if (res == 2)
          return true;
      }

  return addItem(par, Name) ? true : false;
}

KDlgItem_Widget *KDlgEditWidget::addItem(KDlgItem_Base *par, QString Name)
{
  if (!par)
    return 0;

  KDlgItem_Widget *wid = 0;

  #define macro_CreateIfRightOne(a,typ) \
    if (QString(a).upper() == Name.upper()) \
      { \
        wid = (KDlgItem_Widget*)new typ( this, par->getItem() ); \
        if (wid->getProps()->getProp("VarName")->value == "") \
          { \
            typeCounter.increase(a); \
            wid->getProps()->setProp_Value("VarName", typeCounter.countString(a)); \
          } \
      }
  
    macro_CreateIfRightOne("QWidget", KDlgItem_Widget )
    macro_CreateIfRightOne("QPushButton", KDlgItem_PushButton )
    macro_CreateIfRightOne("QLineEdit", KDlgItem_LineEdit )
    macro_CreateIfRightOne("QLabel", KDlgItem_Label )
    macro_CreateIfRightOne("QLCDNumber", KDlgItem_LCDNumber )
    macro_CreateIfRightOne("QCheckBox", KDlgItem_CheckBox )
    macro_CreateIfRightOne("QRadioButton", KDlgItem_RadioButton )
    macro_CreateIfRightOne("QComboBox", KDlgItem_ComboBox )
    macro_CreateIfRightOne("QListBox", KDlgItem_ListBox )
    macro_CreateIfRightOne("QProgressBar", KDlgItem_ProgressBar )
    macro_CreateIfRightOne("QMultiLineEdit", KDlgItem_MultiLineEdit )
    macro_CreateIfRightOne("QSpinBox", KDlgItem_SpinBox )
    macro_CreateIfRightOne("QSlider", KDlgItem_Slider )
    macro_CreateIfRightOne("QScrollBar", KDlgItem_ScrollBar )
    macro_CreateIfRightOne("QGroupBox", KDlgItem_GroupBox )
    macro_CreateIfRightOne("QListView", KDlgItem_ListView )
#if 0
    macro_CreateIfRightOne("KCombo", KDlgItem_KCombo )
#endif
    macro_CreateIfRightOne("KDatePicker", KDlgItem_KDatePicker )
    macro_CreateIfRightOne("KDateTable", KDlgItem_KDateTable )
    macro_CreateIfRightOne("KColorButton", KDlgItem_KColorButton )

    macro_CreateIfRightOne("KLedLamp", KDlgItem_KLedLamp )
    macro_CreateIfRightOne("KProgress", KDlgItem_KProgress )
    macro_CreateIfRightOne("KKeyButton", KDlgItem_KKeyButton )
    macro_CreateIfRightOne("KRestrictedLine", KDlgItem_KRestrictedLine )
    macro_CreateIfRightOne("KTreeList", KDlgItem_KTreeList )
    macro_CreateIfRightOne("KSeparator", KDlgItem_KSeparator )

#undef macro_CreateIfRightOne

  setModified(true);
  setWidgetAdded(true);

  if (!wid)
    return 0;

  par->addChild( wid );
  if (par != main_widget)
    wid->parentWidgetItem = (KDlgItem_Widget*)par;

//  wid->repaintItem();

  if ((pCKDevel) && ((CKDevelop*)pCKDevel)->kdlg_get_items_view())
    ((CKDevelop*)pCKDevel)->kdlg_get_items_view()->addWidgetChilds(main_widget);

  return wid;
}



void KDlgEditWidget::typeCount::clear()
{
  int i;
  for (i = 0; i<64; i++)
    {
      types[i] = "";
      counts[i] = 0;
    }
}

int KDlgEditWidget::typeCount::returnCount(QString type)
{
  int i;
  for (i = 0; i<64; i++)
    if (type.upper() == types[i].upper())
      return counts[i];

  return 0;
}

void KDlgEditWidget::typeCount::addType(QString type)
{
  int i;
  for (i = 0; i<64; i++)
    if ((types[i]=="") && (returnCount(type) == 0))
      {
        types[i] = type.stripWhiteSpace();
        counts[i] = 1;
      }
}

void KDlgEditWidget::typeCount::increase(QString type)
{
  int i;
  for (i = 0; i<64; i++)
    if (type.stripWhiteSpace().upper() == types[i].upper())
      {
        counts[i]++;
        return;
      }

  addType(type);
}

void KDlgEditWidget::typeCount::decrease(QString type)
{
  int i;
  for (i = 0; i<64; i++)
    if (type.upper().stripWhiteSpace() == types[i].upper())
      {
        counts[i]--;
        return;
      }

  addType(type);
}

QString KDlgEditWidget::typeCount::countString(QString type)
{
  return type + QString("_") + QString().setNum(returnCount(type));
}



