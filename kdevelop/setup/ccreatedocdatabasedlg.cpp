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
#include <qlayout.h>
#include <qgrid.h>
#include <qmultilineedit.h>
#include <qapplication.h>

#include <kmessagebox.h>
#include <kfiledialog.h>
//#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kprocess.h>
#include <kiconloader.h>
#include <kbuttonbox.h>
//#include <kconfig.h>

#include "ccreatedocdatabasedlg.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#include <iostream>
//using namespace std;

CCreateDocDatabaseDlg::CCreateDocDatabaseDlg(QWidget *parent, const char *name,KShellProcess* proc,const QString& kdeDocDir, const QString& qtDocDir, bool foundGlimpse,bool foundHtDig, bool bShowIndexingButton) : QWidget(parent,name)
	,start_button(0L)
  ,m_proc(proc)
  ,m_pShellProcessOutput(0L)
  ,m_pShellProcessOutputLines(0L)
  ,m_pShellProcessOutputOKButton(0L)
{
  m_kdeDocDir = kdeDocDir;
  m_qtDocDir = qtDocDir;

  QGridLayout *grid2 = new QGridLayout(this,2,3,0,7);

  //-----search engine group-----
  QButtonGroup* searcheng_ButtonGroup;
  searcheng_ButtonGroup = new QButtonGroup( this, "ButtonGroup_0" );
  searcheng_ButtonGroup->setTitle( i18n("Index Engine") );

  useGlimpse = new QRadioButton( i18n("Glimpse"), searcheng_ButtonGroup );
  if (foundGlimpse)
    useGlimpse->setChecked( true );
  else
    useGlimpse->setEnabled( false );

  useHtDig = new QRadioButton( i18n("ht://Dig"), searcheng_ButtonGroup );
  if (foundGlimpse)
    useHtDig->setChecked( false );
  else if (foundHtDig)
    useHtDig->setChecked( true );

  QGridLayout *grid1 = new QGridLayout(searcheng_ButtonGroup,3,1,15,7);
  grid1->addWidget(useGlimpse,0,0);
  grid1->addWidget(useHtDig,1,0);
  grid2->addWidget(searcheng_ButtonGroup, 0, 0);

  // ------- index size group --------
  QButtonGroup* qtarch_ButtonGroup_1;
  qtarch_ButtonGroup_1 = new QButtonGroup( this, "ButtonGroup_1" );
  qtarch_ButtonGroup_1->setTitle( i18n("Index Size") );

  tiny_radio_button = new QRadioButton(  qtarch_ButtonGroup_1, "RadioButton_2" );
  tiny_radio_button->setText(i18n("Tiny size") );
  tiny_radio_button->setChecked( TRUE );

  small_radio_button = new QRadioButton( qtarch_ButtonGroup_1, "RadioButton_3" );
  small_radio_button->setText(i18n("Small size") );

  medium_radio_button = new QRadioButton( qtarch_ButtonGroup_1, "RadioButton_4" );
  medium_radio_button->setText(i18n("Medium size") );

  grid1 = new QGridLayout(qtarch_ButtonGroup_1,3,1,15,7);
  grid1->addWidget(tiny_radio_button,0,0);
  grid1->addWidget(small_radio_button,1,0);
  grid1->addWidget(medium_radio_button,2,0);
  grid2->addWidget(qtarch_ButtonGroup_1, 0, 1);

  // ------- index options group --------
  QWidget* wdg = new QWidget(this);
  QButtonGroup* qtarch_ButtonGroup_3;
  qtarch_ButtonGroup_3 = new QButtonGroup( wdg, "ButtonGroup_3" );
  qtarch_ButtonGroup_3->setTitle( i18n("Index Options") );

  qt_checkbox = new QCheckBox( qtarch_ButtonGroup_3, "CheckBox_3" );
  qt_checkbox->setText( i18n("Index the QT documentation") );
  qt_checkbox->setChecked( TRUE );

  kde_checkbox = new QCheckBox( qtarch_ButtonGroup_3, "CheckBox_1" );
  kde_checkbox->setText(i18n("Index the KDE documentation") );
  kde_checkbox->setChecked( TRUE );

	if (bShowIndexingButton) {
	  start_button  = new QPushButton(i18n("Start Indexing"), wdg);
  	start_button->setDefault(true);
	}

  grid1 = new QGridLayout(qtarch_ButtonGroup_3,2,1,15,7);
  grid1->addWidget(qt_checkbox,0,0);
  grid1->addWidget(kde_checkbox,1,0);
  QVBoxLayout* vl = new QVBoxLayout(wdg,0,7);
  vl->addWidget(qtarch_ButtonGroup_3);
	if (bShowIndexingButton)
	  vl->addWidget(start_button);
  grid2->addWidget(wdg, 0, 2);

  // ------- additional dirs group --------------
  QButtonGroup* qtarch_ButtonGroup_2;
  qtarch_ButtonGroup_2 = new QButtonGroup( this, "ButtonGroup_2" );
  qtarch_ButtonGroup_2->setTitle(i18n("Additional Directories to Index"));

  dir_edit = new QLineEdit( qtarch_ButtonGroup_2, "LineEdit_1" );
  dir_edit->setText( "" );
  dir_edit->setEchoMode( QLineEdit::Normal );
  dir_edit->setFrame( TRUE );

  dir_button = new QPushButton( qtarch_ButtonGroup_2, "PushButton_3" );
  QPixmap pix = SmallIcon("fileopen");
  dir_button->setPixmap(pix);

  dir_listbox = new QListBox(  qtarch_ButtonGroup_2, "ListBox_1" );
  dir_listbox->setMultiSelection( FALSE );

  add_button = new QPushButton(i18n("add"), qtarch_ButtonGroup_2);
  remove_button = new QPushButton(i18n("remove"), qtarch_ButtonGroup_2);

  grid1 = new QGridLayout(qtarch_ButtonGroup_2,3,2,15,7);
  grid1->addWidget(dir_edit,0,0);
  grid1->addWidget(dir_button,0,1);
  grid1->addWidget(add_button,1,1);
  grid1->addWidget(remove_button,2,1);
  grid1->addMultiCellWidget(dir_listbox,1,2,0,0);
  grid2->addMultiCellWidget(qtarch_ButtonGroup_2,1,1,0,2);

  resize(sizeHint());

 /*****************Connections******************/
 if (bShowIndexingButton)
	 connect(start_button,SIGNAL(clicked()),SLOT(slotOkClicked()));
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

  createShellProcessOutputWidget();
}

CCreateDocDatabaseDlg::~CCreateDocDatabaseDlg()
{
}

void CCreateDocDatabaseDlg::slotOkClicked()
{
  if(!QFile::exists(m_kdeDocDir + "/kdeui/KDialog.html") && kde_checkbox->isChecked()){
    KMessageBox::error(0,i18n("The KDE-Documentation-Path isn't set correctly."),i18n("No Database Created!"));
    return;
  }
  if(!QFile::exists(m_qtDocDir + "/qtabbar.html") && qt_checkbox->isChecked()){
    KMessageBox::error(0,i18n("The Qt-Documentation-Path isn't set correctly."),i18n("No Database Created!"));
    return;
  }

  QDir dir(locateLocal("data", ""));
  dir.mkdir("kdevelop");

  QString dirs;
  if(kde_checkbox->isChecked()){
    dirs = dirs + m_kdeDocDir;
  }
  if(qt_checkbox->isChecked()){
    dirs = dirs + " " +  m_qtDocDir;
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
  else if (medium_radio_button->isChecked()){
    size_str = " -b ";
  }
  
  m_proc->clearArguments();
  if (useGlimpse->isChecked())
  {
    *m_proc <<  "find "+ dirs +" -name '*.html' | glimpseindex " +
                    size_str +" -F -X -H "+ locateLocal("appdata","");
    m_proc->start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  }
  else if (useHtDig->isChecked())
  {
    QString htdigConf = locate("appdata", "tools/htdig.conf");
    if (!htdigConf.isEmpty()) {
      *m_proc <<  "find " +
                dirs +
                " -name '*.html' | awk 'OFS=\"\"; {print \"file://localhost\", $0}' | htdig -v -s -c " +
                htdigConf + " - ; htmerge -v -s -c " + htdigConf;
      m_proc->start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
      slotShowToolProcessOutputDlg();
    }
    else {
      KMessageBox::sorry(this, i18n("Couldn't find a htdig.conf"), i18n("htdig Execution Error"));
    }
  }
}

void CCreateDocDatabaseDlg::slotAddButtonClicked()
{
  QString str = dir_edit->text();

  if(str != "" ){
    dir_listbox->insertItem(str,0);
    dir_edit->clear();
  }
  
}

void CCreateDocDatabaseDlg::slotRemoveButtonClicked()
{
  dir_listbox->removeItem(dir_listbox->currentItem());
  
}

void CCreateDocDatabaseDlg::slotDirButtonClicked()
{
  QString name=KFileDialog::getExistingDirectory(dir_edit->text(),0,i18n("Select Directory..."));
  if(!name.isEmpty()){
    dir_edit->setText(name);
  }
}

void CCreateDocDatabaseDlg::createShellProcessOutputWidget()
{
  m_pShellProcessOutput = new QDialog(this, "shell_process_output_dlg", true);
  m_pShellProcessOutput->setCaption(i18n("Creating the KDE Documentation"));
  QVBoxLayout* pVL = new QVBoxLayout(m_pShellProcessOutput, 15, 7);
  QLabel* pLabel = new QLabel(i18n("Wait until the process has finished:"), m_pShellProcessOutput);
  m_pShellProcessOutputLines = new QMultiLineEdit(m_pShellProcessOutput);
  KButtonBox *bb = new KButtonBox( m_pShellProcessOutput );
  bb->addStretch();
  m_pShellProcessOutputOKButton =bb->addButton( i18n("&OK") );
  m_pShellProcessOutputOKButton->setDefault( true );
  bb->addStretch();

  pVL->addWidget(pLabel);
  pVL->addWidget(m_pShellProcessOutputLines);
  pVL->addWidget(bb);

  m_pShellProcessOutput->resize(500, 400);

  QObject::connect(m_pShellProcessOutputOKButton, SIGNAL(clicked()), m_pShellProcessOutput, SLOT(accept()));
  QObject::connect(m_proc,SIGNAL(receivedStdout(KProcess*,char*,int)), this, SLOT(slotReceivedStdout(KProcess*,char*,int)) );
  QObject::connect(m_proc,SIGNAL(receivedStderr(KProcess*,char*,int)), this, SLOT(slotReceivedStderr(KProcess*,char*,int)) );
  QObject::connect(m_proc,SIGNAL(processExited(KProcess*)), this, SLOT(slotProcessExited(KProcess*) )) ;
}

void CCreateDocDatabaseDlg::slotShowToolProcessOutputDlg()
{
  m_pShellProcessOutputOKButton->setEnabled(false);
  m_pShellProcessOutputLines->clear();
  QApplication::setOverrideCursor(WaitCursor);
  emit indexingStartedNow();
	if (start_button)
	  start_button->setEnabled(false);
  m_pShellProcessOutput->exec();
}

void CCreateDocDatabaseDlg::slotReceivedStdout(KProcess*,char* buffer,int count)
{
  QCString test(buffer, count);
  m_pShellProcessOutputLines->insertLine(test);
  m_pShellProcessOutputLines->setCursorPosition(m_pShellProcessOutputLines->numLines(), 0);
}

void CCreateDocDatabaseDlg::slotReceivedStderr(KProcess*,char* buffer, int count)
{
  QCString test(buffer, count);
  m_pShellProcessOutputLines->insertLine(test);
  m_pShellProcessOutputLines->setCursorPosition(m_pShellProcessOutputLines->numLines(), 0);
}

void CCreateDocDatabaseDlg::slotProcessExited(KProcess*)
{
  m_pShellProcessOutputLines->insertLine("");
  m_pShellProcessOutputLines->insertLine("Finished!");
  m_pShellProcessOutputLines->setCursorPosition(m_pShellProcessOutputLines->numLines(), 0);
  m_pShellProcessOutputOKButton->setEnabled(true);
	if (start_button)
	  start_button->setEnabled(true);
  QApplication::restoreOverrideCursor();

  if (useGlimpse->isChecked())
    emit indexingFinished("glimpse");
  else
    emit indexingFinished("htdig");
}

void CCreateDocDatabaseDlg::setQtDocDir(const QString& qtDocDir)
{
  m_qtDocDir = qtDocDir;
}

#include "ccreatedocdatabasedlg.moc"
