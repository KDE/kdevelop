/***************************************************************************
                          ccreatedocdatabasedlg.cpp  -  description                              
                             -------------------                                         

    begin                : Sat Jan 9 1999                                           
    copyright            : (C) 1999 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "ccreatedocdatabasedlg.h"

#include <kmessagebox.h>
#include <kfiledialog.h>
//#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kprocess.h>
//#include <kconfig.h>

#include <qdir.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#include <iostream.h>

CCreateDocDatabaseDlg::CCreateDocDatabaseDlg(QWidget *parent, const char *name,KShellProcess* proc,KConfig* config,bool foundGlimpse,bool foundHtDig) : QDialog(parent,name,true) {

  setCaption(i18n("Create Search Database..."));
  this->proc = proc;
  this->conf = config;

	QButtonGroup *bg = new QButtonGroup( this, 0 );
  bg->setFrameStyle( QFrame::NoFrame );
  bg->setExclusive( TRUE );
  bg->setGeometry( 20, 20, 380, 50 );

  QLabel* lbl;
  lbl = new QLabel( bg, "index_engine" );
  lbl->setGeometry( 0, 0, 100, 20 );
  lbl->setText(i18n("Index engine :") );
  lbl->setAlignment( 289 );
  lbl->setMargin( -1 );

  useGlimpse = new QRadioButton( i18n("Glimpse"), bg );
  useGlimpse->setGeometry( 140, 0, 80, 20 );
  if (foundGlimpse)
    useGlimpse->setChecked( true );
  else
    useGlimpse->setEnabled( false );

  useHtDig = new QRadioButton( i18n("ht://Dig"), bg );
  useHtDig->setGeometry( 220, 0, 80, 20 );
  if (foundGlimpse)
    useHtDig->setChecked( false );
  else if (foundHtDig)
    useHtDig->setChecked( true );

	QButtonGroup* qtarch_ButtonGroup_1;
	qtarch_ButtonGroup_1 = new QButtonGroup( this, "ButtonGroup_1" );
	qtarch_ButtonGroup_1->setGeometry( 20, 50, 190, 120 );
	qtarch_ButtonGroup_1->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_1->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_1->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_1->setFrameStyle( 49 );
	qtarch_ButtonGroup_1->setTitle( i18n("Index Size") );
	qtarch_ButtonGroup_1->setAlignment( 1 );

	tiny_radio_button = new QRadioButton( this, "RadioButton_2" );
	tiny_radio_button->setGeometry( 40, 70, 100, 30 );
	tiny_radio_button->setMinimumSize( 0, 0 );
	tiny_radio_button->setMaximumSize( 32767, 32767 );
	tiny_radio_button->setFocusPolicy( QWidget::TabFocus );
	tiny_radio_button->setBackgroundMode( QWidget::PaletteBackground );
	tiny_radio_button->setFontPropagation( QWidget::NoChildren );
	tiny_radio_button->setPalettePropagation( QWidget::NoChildren );
	tiny_radio_button->setText(i18n("tiny size") );
	tiny_radio_button->setAutoRepeat( FALSE );
	tiny_radio_button->setAutoResize( FALSE );
	tiny_radio_button->setChecked( TRUE );

	small_radio_button = new QRadioButton( this, "RadioButton_3" );
	small_radio_button->setGeometry( 40, 100, 100, 30 );
	small_radio_button->setMinimumSize( 0, 0 );
	small_radio_button->setMaximumSize( 32767, 32767 );
	small_radio_button->setFocusPolicy( QWidget::TabFocus );
	small_radio_button->setBackgroundMode( QWidget::PaletteBackground );
	small_radio_button->setFontPropagation( QWidget::NoChildren );
	small_radio_button->setPalettePropagation( QWidget::NoChildren );
	small_radio_button->setText(i18n("small size") );
	small_radio_button->setAutoRepeat( FALSE );
	small_radio_button->setAutoResize( FALSE );

	medium_radio_button = new QRadioButton( this, "RadioButton_4" );
	medium_radio_button->setGeometry( 40, 130, 100, 30 );
	medium_radio_button->setMinimumSize( 0, 0 );
	medium_radio_button->setMaximumSize( 32767, 32767 );
	medium_radio_button->setFocusPolicy( QWidget::TabFocus );
	medium_radio_button->setBackgroundMode( QWidget::PaletteBackground );
	medium_radio_button->setFontPropagation( QWidget::NoChildren );
	medium_radio_button->setPalettePropagation( QWidget::NoChildren );
	medium_radio_button->setText(i18n("medium size") );
	medium_radio_button->setAutoRepeat( FALSE );
	medium_radio_button->setAutoResize( FALSE );

  QButtonGroup* qtarch_ButtonGroup_3;
	qtarch_ButtonGroup_3 = new QButtonGroup( this, "ButtonGroup_3" );
	qtarch_ButtonGroup_3->setGeometry( 230, 50, 240, 120 );
	qtarch_ButtonGroup_3->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_3->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_3->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_3->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_3->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_3->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_3->setFrameStyle( 49 );
	qtarch_ButtonGroup_3->setTitle( i18n("Index Options") );
	qtarch_ButtonGroup_3->setAlignment( 1 );

	qt_checkbox = new QCheckBox( this, "CheckBox_3" );
	qt_checkbox->setGeometry( 250, 70, 210, 30 );
	qt_checkbox->setMinimumSize( 0, 0 );
	qt_checkbox->setMaximumSize( 32767, 32767 );
	qt_checkbox->setFocusPolicy( QWidget::TabFocus );
	qt_checkbox->setBackgroundMode( QWidget::PaletteBackground );
	qt_checkbox->setFontPropagation( QWidget::NoChildren );
	qt_checkbox->setPalettePropagation( QWidget::NoChildren );
	qt_checkbox->setText( i18n("index the QT documentation") );
	qt_checkbox->setAutoRepeat( FALSE );
	qt_checkbox->setAutoResize( FALSE );
	qt_checkbox->setChecked( TRUE );

	kde_checkbox = new QCheckBox( this, "CheckBox_1" );
	kde_checkbox->setGeometry( 250, 100, 210, 30 );
	kde_checkbox->setMinimumSize( 0, 0 );
	kde_checkbox->setMaximumSize( 32767, 32767 );
	kde_checkbox->setFocusPolicy( QWidget::TabFocus );
	kde_checkbox->setBackgroundMode( QWidget::PaletteBackground );
	kde_checkbox->setFontPropagation( QWidget::NoChildren );
	kde_checkbox->setPalettePropagation( QWidget::NoChildren );
	kde_checkbox->setText(i18n("index the KDE documentation") );
	kde_checkbox->setAutoRepeat( FALSE );
	kde_checkbox->setAutoResize( FALSE );
	kde_checkbox->setChecked( TRUE );

	QButtonGroup* qtarch_ButtonGroup_2;
	qtarch_ButtonGroup_2 = new QButtonGroup( this, "ButtonGroup_2" );
	qtarch_ButtonGroup_2->setGeometry( 20, 180, 450, 210 );
	qtarch_ButtonGroup_2->setMinimumSize( 0, 0 );
	qtarch_ButtonGroup_2->setMaximumSize( 32767, 32767 );
	qtarch_ButtonGroup_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_ButtonGroup_2->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_ButtonGroup_2->setFontPropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_ButtonGroup_2->setFrameStyle( 49 );
	qtarch_ButtonGroup_2->setTitle(i18n("additional directories to index"));
	qtarch_ButtonGroup_2->setAlignment( 1 );

	dir_edit = new QLineEdit( this, "LineEdit_1" );
	dir_edit->setGeometry( 40, 210, 280, 30 );
	dir_edit->setMinimumSize( 0, 0 );
	dir_edit->setMaximumSize( 32767, 32767 );
	dir_edit->setFocusPolicy( QWidget::StrongFocus );
	dir_edit->setBackgroundMode( QWidget::PaletteBase );
	dir_edit->setFontPropagation( QWidget::NoChildren );
	dir_edit->setPalettePropagation( QWidget::NoChildren );
	dir_edit->setText( "" );
	dir_edit->setMaxLength( 32767 );
	dir_edit->setEchoMode( QLineEdit::Normal );
	dir_edit->setFrame( TRUE );

	dir_button = new QPushButton( this, "PushButton_3" );
	dir_button->setGeometry( 340, 210, 30, 30 );
	dir_button->setMinimumSize( 0, 0 );
	dir_button->setMaximumSize( 32767, 32767 );
	dir_button->setFocusPolicy( QWidget::TabFocus );
	dir_button->setBackgroundMode( QWidget::PaletteBackground );
	dir_button->setFontPropagation( QWidget::NoChildren );
	dir_button->setPalettePropagation( QWidget::NoChildren );
	QPixmap pix = BarIcon("open");
	dir_button->setPixmap(pix);
	dir_button->setAutoRepeat( FALSE );
	dir_button->setAutoResize( FALSE );

	dir_listbox = new QListBox( this, "ListBox_1" );
	dir_listbox->setGeometry( 40, 260, 280, 110 );
	dir_listbox->setMinimumSize( 0, 0 );
	dir_listbox->setMaximumSize( 32767, 32767 );
	dir_listbox->setFocusPolicy( QWidget::StrongFocus );
	dir_listbox->setBackgroundMode( QWidget::PaletteBase );
	dir_listbox->setFontPropagation( QWidget::SameFont );
	dir_listbox->setPalettePropagation( QWidget::SameFont );
	dir_listbox->setFrameStyle( 51 );
	dir_listbox->setLineWidth( 2 );
	dir_listbox->setMultiSelection( FALSE );

	add_button = new QPushButton( this, "PushButton_4" );
	add_button->setGeometry( 340, 260, 100, 30 );
	add_button->setMinimumSize( 0, 0 );
	add_button->setMaximumSize( 32767, 32767 );
	add_button->setFocusPolicy( QWidget::TabFocus );
	add_button->setBackgroundMode( QWidget::PaletteBackground );
	add_button->setFontPropagation( QWidget::NoChildren );
	add_button->setPalettePropagation( QWidget::NoChildren );
	add_button->setText(i18n("add") );
	add_button->setAutoRepeat( FALSE );
	add_button->setAutoResize( FALSE );

	remove_button = new QPushButton( this, "PushButton_5" );
	remove_button->setGeometry( 340, 300, 100, 30 );
	remove_button->setMinimumSize( 0, 0 );
	remove_button->setMaximumSize( 32767, 32767 );
	remove_button->setFocusPolicy( QWidget::TabFocus );
	remove_button->setBackgroundMode( QWidget::PaletteBackground );
	remove_button->setFontPropagation( QWidget::NoChildren );
	remove_button->setPalettePropagation( QWidget::NoChildren );
	remove_button->setText(i18n("remove") );
	remove_button->setAutoRepeat( FALSE );
	remove_button->setAutoResize( FALSE );

	qtarch_ButtonGroup_3->insert( kde_checkbox );
	qtarch_ButtonGroup_3->insert( qt_checkbox );

	qtarch_ButtonGroup_2->insert( dir_button );
	qtarch_ButtonGroup_2->insert( add_button );
	qtarch_ButtonGroup_2->insert( remove_button );

	qtarch_ButtonGroup_1->insert( tiny_radio_button );
	qtarch_ButtonGroup_1->insert( small_radio_button );
	qtarch_ButtonGroup_1->insert( medium_radio_button );
	
	ok_button = new QPushButton( this, "PushButton_1" );
	ok_button->setGeometry( 100, 400, 100, 30 );
	ok_button->setMinimumSize( 0, 0 );
	ok_button->setMaximumSize( 32767, 32767 );
	ok_button->setFocusPolicy( QWidget::TabFocus );
	ok_button->setBackgroundMode( QWidget::PaletteBackground );
	ok_button->setFontPropagation( QWidget::NoChildren );
	ok_button->setPalettePropagation( QWidget::NoChildren );
	ok_button->setText( i18n("OK") );
	ok_button->setAutoRepeat( FALSE );
	ok_button->setAutoResize( FALSE );
	ok_button->setDefault(true);
	
	cancel_button = new QPushButton( this, "PushButton_2" );
	cancel_button->setGeometry( 280, 400, 100, 30 );
	cancel_button->setMinimumSize( 0, 0 );
	cancel_button->setMaximumSize( 32767, 32767 );
	cancel_button->setFocusPolicy( QWidget::TabFocus );
	cancel_button->setBackgroundMode( QWidget::PaletteBackground );
	cancel_button->setFontPropagation( QWidget::NoChildren );
	cancel_button->setPalettePropagation( QWidget::NoChildren );
	cancel_button->setText( i18n("Cancel") );
	cancel_button->setAutoRepeat( FALSE );
	cancel_button->setAutoResize( FALSE );

	resize( 490,440 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );



	/*****************Connections******************/
	connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
	connect(ok_button,SIGNAL(clicked()),SLOT(slotOkClicked()));
	connect(add_button,SIGNAL(clicked()),SLOT(slotAddButtonClicked()));
	connect(remove_button,SIGNAL(clicked()),SLOT(slotRemoveButtonClicked()));
	connect(dir_button,SIGNAL(clicked()),SLOT(slotDirButtonClicked()));

	dir_edit->setFocus();

	/*doc*/
	QWhatsThis::add(medium_radio_button,
	i18n("builds a medium-size index (20-30% of the size\n"
	     "of all files), allowing faster search."));
	QWhatsThis::add(small_radio_button,
			i18n("Build a small index rather than tiny (meaning 7-9%\n"
			     "of the sizes of all files - your mileage may vary)\n"
			     "allowing faster search."));
	QWhatsThis::add(tiny_radio_button,
			i18n("a tiny index (2-3% of the total size of all files)"));
	
	
	
}
CCreateDocDatabaseDlg::~CCreateDocDatabaseDlg(){
}

void CCreateDocDatabaseDlg::slotOkClicked(){
  conf->setGroup("Doc_Location");
  QString filename = conf->readEntry("doc_kde", KDELIBS_DOCDIR) +"/kdeui/KDialog.html";
  if(!QFile::exists(filename) && kde_checkbox->isChecked()){
    KMessageBox::error(0,i18n("The KDE-Documentation-Path isn't set correctly."),i18n("No Database created!"));
    return;
  }
  filename = conf->readEntry("doc_qt", QT_DOCDIR) +"/qtabbar.html";
  if(!QFile::exists(filename) && qt_checkbox->isChecked()){
    KMessageBox::error(0,i18n("The Qt-Documentation-Path isn't set correctly."),i18n("No Database created!"));
    return;
  }
  
  QDir dir(locateLocal("data", ""));
  dir.mkdir("kdevelop");
 
  QString kde_doc_dir = conf->readEntry("doc_kde", KDELIBS_DOCDIR);
  QString qt_doc_dir = conf->readEntry("doc_qt", QT_DOCDIR);
  
  QString dirs;
  if(kde_checkbox->isChecked()){
    dirs = dirs + kde_doc_dir;
  }
  if(qt_checkbox->isChecked()){
    dirs = dirs + " " +  qt_doc_dir;
  }
  // added for documentation search in the kdevelop html directory
  dirs= dirs + " "+ locate("html", "default/kdevelop");

  uint count = dir_listbox->count();
  uint i;
  for(i=0;i<count;i++){
    dirs = dirs + " ";
    dirs = dirs + dir_listbox->text(i);
  }
  
  QString size_str;
  if (small_radio_button->isChecked()){
    size_str = " -o ";
  }
  if (medium_radio_button->isChecked()){
    size_str = " -b ";
  }
  
  proc->clearArguments();
  if (useGlimpse->isChecked())
  {
    *proc <<  "find "+ dirs +" -name '*.html' | glimpseindex " + size_str +" -F -X -H "+ locateLocal("data","");
    proc->start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
    accept();
  }
  if (useHtDig->isChecked())
  {
    *proc <<  "find " +
                dirs +
                " -name '*.html' | awk 'OFS=\"\"; {print \"file://localhost\", $0}' | htdig -v -s -c " +
                locate("appdata", "tools/htdig.conf") +
                " - ; htmerge -v -s -c " +
                locate("appdata", "tools/htdig.conf");
    proc->start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
    accept();
  }
}
void CCreateDocDatabaseDlg::slotAddButtonClicked(){
  QString str = dir_edit->text();

  if(str != "" ){
    dir_listbox->insertItem(str,0);
    dir_edit->clear();
  }
  
}
void CCreateDocDatabaseDlg::slotRemoveButtonClicked(){
  dir_listbox->removeItem(dir_listbox->currentItem());
  
}
void CCreateDocDatabaseDlg::slotDirButtonClicked(){
  QString name=KFileDialog::getExistingDirectory(dir_edit->text(),0,i18n("Select Directory..."));
  if(!name.isEmpty()){
    dir_edit->setText(name);
  }
}
