/***************************************************************************
                          cupdatekdedocdlg.cpp  -  description
                             -------------------

    begin                : Mon Nov 9 1998
    copyright            : (C) 1998 by Sandy Meier
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

#include <qbuttongroup.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qgrid.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qapplication.h>

#include <kconfig.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <kbuttonbox.h>

#include "cupdatekdedocdlg.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


CUpdateKDEDocDlg::CUpdateKDEDocDlg(KShellProcess* proc, const QString& kdeDocDir, const QString& qtDocDir, QWidget *parent, bool bShowCancelButton, const char *name) : QWidget(parent,name)
  ,m_proc(proc)
  ,m_pShellProcessOutput(0L)
  ,m_pShellProcessOutputLines(0L)
  ,m_pShellProcessOutputOKButton(0L)
{
  kde_doc_path = kdeDocDir;
  qt_doc_path = qtDocDir;

  QVBoxLayout* vl1 = new QVBoxLayout(this, 15, 7);

  QWidget* wdg1 = new QWidget(this);
  source_label = new QLabel( wdg1, "source_label" );
  source_label->setText(i18n("new KDE Libs sources path:") );
  source_edit = new QLineEdit( wdg1, "source_edit" );
  source_edit->setText( QDir::homeDirPath() );
  source_button = new QPushButton( wdg1, "source_button" );
  QPixmap pix = SmallIcon("fileopen");
  source_button->setPixmap(pix);

  QHBoxLayout* hl1 = new QHBoxLayout(wdg1, 0, 7);
  hl1->addWidget(source_label);
  hl1->addWidget(source_edit);
  hl1->addWidget(source_button);
  vl1->addWidget(wdg1);

  QWidget* wdg2 = new QWidget(this);
  install_box = new QButtonGroup( wdg2, "install_box" );
  install_box->setTitle(i18n("Choose Installation Mode") );
  QHBoxLayout* hl2 = new QHBoxLayout(wdg2);
  vl1->addWidget(wdg2);
  hl2->addWidget(install_box);

  del_recent_radio_button = new QRadioButton( install_box, "del_recent_radio_button" );
  del_recent_radio_button->setText(i18n("Delete old Documentation and install to recent Documentation path"));
  del_recent_radio_button->setChecked( TRUE );
  QWhatsThis::add(del_recent_radio_button,
      i18n("Checking this will delete the current documentation\n"
     "and replace it with the new generated documentation\n"
     "in the same path."));

  del_new_radio_button = new QRadioButton( install_box, "del_new_radio_button" );
  del_new_radio_button->setText(i18n("Delete old Documentation and install to new Documentation path") );
  QWhatsThis::add(del_new_radio_button,
      i18n("Checking this will delete the current documentation\n"
     "and lets you choose a path in the input field below\n"
     "where the new generated documentation will be"
     "installed."));

  leave_new_radio_button = new QRadioButton( install_box, "leave_new_radio_button" );
  leave_new_radio_button->setText(i18n("Leave old Documention untouched and install to new Documention path") );
  QWhatsThis::add(leave_new_radio_button,
      i18n("This doesn't delete your current documentation, leaves it\n"
     "where it is now and you can select a new path for the new kdelibs\n"
     "documentation. CAUTION: Don't insert the same path as\n"
     "for your recent documentation - this may mess up\n"
     "the documentation by mixing old and new files!"));

  QVBoxLayout* vl2 = new QVBoxLayout(install_box, 15, 7);
  vl2->addWidget(del_recent_radio_button);
  vl2->addWidget(del_new_radio_button);
  vl2->addWidget(leave_new_radio_button);

  QWidget* wdg3 = new QWidget(this);
  doc_label = new QLabel( wdg3, "doc_label" );
  doc_label->setText(i18n("new KDE Libs Documentation path:") );
  doc_label->setEnabled(false);
  doc_edit = new QLineEdit( wdg3, "doc_edit" );
  doc_edit->setText(kde_doc_path);
  doc_edit->setEnabled(false);
  doc_button = new QPushButton( wdg3, "doc_button" );
  doc_button->setPixmap(pix);
  doc_button->setEnabled(false);

  QHBoxLayout* hl3 = new QHBoxLayout(wdg3, 0, 7);
  hl3->addWidget(doc_label);
  hl3->addWidget(doc_edit);
  hl3->addWidget(doc_button);
  vl1->addWidget(wdg3);

  QString sourceHelp = i18n("Insert the path to the current\n"
                     "KDE-Libs sourcecodes here. This is\n"
                     "where you have unpacked e.g. a kdelibs\n"
                     "snapshot a la /snapshot/kdelibs.");
  QWhatsThis::add(source_label, sourceHelp);
  QWhatsThis::add(source_edit, sourceHelp);
  QWhatsThis::add(source_button, sourceHelp);

  QString docMsg = i18n("Insert the path where you want to have\n"
                          "the new generated documentation installed\n"
                          "Note: the path information in Setup will\n"
                          "be updated automatically, you don't have\n"
                          "to change them to the new doc path.");
  QWhatsThis::add(doc_label, docMsg);
  QWhatsThis::add(doc_edit, docMsg);
  QWhatsThis::add(doc_button, docMsg);

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  create_button =bb->addButton( i18n("Create") );
  create_button->setDefault( true );
  if (bShowCancelButton)
     cancel_button = bb->addButton( i18n("Cancel") );    
  
  vl1->addWidget(bb);

  bUpdated=false;

  setFixedHeight(sizeHint().height());

  connect(create_button,SIGNAL(clicked()),SLOT(OK()));
  connect(leave_new_radio_button,SIGNAL(clicked()),SLOT(slotLeaveNewRadioButtonClicked()));
  connect(del_new_radio_button,SIGNAL(clicked()),SLOT(slotDelNewRadioButtonClicked()));
  connect(del_recent_radio_button,SIGNAL(clicked()),SLOT(slotDelRecentRadioButtonClicked()));

  connect(doc_button,SIGNAL(clicked()),SLOT(slotDocButtonClicked()));
  connect(source_button,SIGNAL(clicked()),SLOT(slotSourceButtonClicked()));

  createShellProcessOutputWidget();
}


CUpdateKDEDocDlg::~CUpdateKDEDocDlg(){
}


void CUpdateKDEDocDlg::OK(){
  KShellProcess proc_rm;
  QString kdelibs_path = source_edit->text();

  if(kdelibs_path.right(1) != "/")
    kdelibs_path = kdelibs_path +"/";

  // check if path (TO GENERATE the doc from the sources) is set correctly
  QString kde_testfile=kdelibs_path+"kdoc.rules";
  if(!QFileInfo(kde_testfile).exists())
  {
    KMessageBox::error(this,i18n("The chosen path for the KDE-Libs does not lead to the KDE Libraries.\n\n"
                   "Please choose the correct path!\n\n"
                   "This is where you have unpacked e.g. a kdelibs snapshot a la /snapshot/kdelibs."),
                   i18n("The Selected Path is not Correct!"));
    return;
  }

  QString new_doc_path = kde_doc_path;
  if(!del_recent_radio_button->isChecked())
  { // not recent doc path
    new_doc_path = doc_edit->text();
  }

  if(new_doc_path.right(1) != "/")
    new_doc_path += "/";

  if(kde_doc_path.right(1) != "/")
    kde_doc_path += "/";

  QDir().mkdir(new_doc_path);
  if(!QFileInfo(new_doc_path).isWritable())
  {
    KMessageBox::error(this,
                        i18n("You need write permission to create\n"
                          "the documentation in\n%1").arg(new_doc_path),
                  i18n("Error in Creating Documentation!"));
    return;
  }

  if(!leave_new_radio_button->isChecked())
  {
    // ok,let's delete it,attentation!!!
    proc_rm.clearArguments();
    if(!QFileInfo(kde_doc_path).exists())
    {
      KMessageBox::error(this,
                          i18n("The old documentation path\n%1"
                                "\ndoesn't exist anymore."
                                "\nProcess will continue without deletion...").arg(kde_doc_path),
                          i18n("Old Documentation Deletion!"));
    }
    else
    {
      if(!QFileInfo(kde_doc_path).isWritable())
      {
        KMessageBox::error(this,
                    i18n("You have no write permission to delete\n"
                            "the old documentation in\n%1"
                            "\nProcess will continue without deletion...").arg(kde_doc_path),
                    i18n("Old Documentation Deletion!"));
      }
      else
      {
        if (QDir::setCurrent(kde_doc_path))
        {
          QString command;
          // protect the rest of the files in the directory...
          //   maybe someone installs the htmls in the source dir of the
          //   kdelib
          command=  "rm -f -r kdoc-reference/;rm -f -r kdecore/*.htm*;"
                    "rm -f -r kdeui/*.htm*;rm -f -r kio/*.htm*;"
                    "rm -f -r kimgio/*.htm*;rm -f -r mediatool/*.htm*;"
                    "rm -f -r kdeutils/*.htm*;"
                    "rm -f -r jscript/*.htm*;rm -f -r kfile/*.htm*;"
                    "rm -f -r khtml/*.htm*;rm -f -r kfmlib/*.htm*;"
                    "rm -f -r kab/*.htm*;rm -f -r kspell/*.htm*;"
          // use here rmdir (so the directory won't be deleted if there are other
          //  files than the documentation
                    "rmdir kdecore; rmdir kdeui;rmdir kio;"
                    "rmdir kimgio; rmdir mediatool; rmdir kdeutils;"
                    "rmdir jscript; rmdir kfile; rmdir khtml; rmdir kfmlib;"
                    "rmdir kab; rmdir kspell";
          //  if the old path and the new doc path differs then
          //  delete the old doc dir
          if (kde_doc_path!=new_doc_path)
            command += "; cd ~; rmdir "+kde_doc_path;

          proc_rm << command;
          proc_rm.start(KShellProcess::Block,KShellProcess::AllOutput);
        }
      }
    }
  }

  m_proc->clearArguments();
  QDir::setCurrent(kdelibs_path);

  QString qtPath = qt_doc_path;
  if(qtPath.right(1) != "/")
    qtPath += "/";

  bool qt_test=false;
  QString qt_testfile=qtPath+"classes.html";
  if(QFileInfo(qt_testfile).exists())
    qt_test=true;

  if(! qt_test)
  {
    int qt_set=KMessageBox::questionYesNo(this,i18n("The Qt-Documentation path is not set correctly.\n"
                                                "If you want your KDE-library documentation to\n"
                                                "be cross-referenced to the Qt-library, you have\n"
                                                "to set the correct path to your Qt-library\n"
                                                "documentation first.\n"
                                                "Do you want to set the Qt-Documentation path first?"));
    if (qt_set==KMessageBox::Yes)
      return;
  }
  else
  {
    QString qt_kdoc=new_doc_path+"kdoc-reference/qt.kdoc.gz";
    if (QFileInfo(qt_kdoc).exists())
      QFile::remove(qt_kdoc);

    // don´t try to create the qt.kdoc.gz file if it isn't removed...
    //  it would block KDevelop
    if (!QFileInfo(qt_kdoc).exists())
    {
      // try to create, if doesn´t qt2kdoc would fail
      QDir().mkdir(new_doc_path+"kdoc-reference");

      *m_proc << "qt2kdoc";
      *m_proc << "--url=file:"  + qtPath;
      *m_proc << "--outdir="   + new_doc_path+"kdoc-reference";
      *m_proc << "--compress";
      *m_proc << qtPath;
      *m_proc << ";\n";
    }
  }

  *m_proc << "makekdedoc";
  *m_proc << "--libdir="    + new_doc_path+"kdoc-reference";
  *m_proc << "--outputdir=" + new_doc_path;
  *m_proc << "--srcdir="   + kdelibs_path;


  m_proc->start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  bUpdated=true;
  kde_doc_path=new_doc_path; // all went ok... so set the new doc_path

  slotShowToolProcessOutputDlg();
}


void CUpdateKDEDocDlg::slotLeaveNewRadioButtonClicked(){
  doc_button->setEnabled(true);
  doc_edit->setEnabled(true);
  doc_label->setEnabled(true);
}


void CUpdateKDEDocDlg::slotDelNewRadioButtonClicked(){
  doc_button->setEnabled(true);
  doc_edit->setEnabled(true);
  doc_label->setEnabled(true);
}


void CUpdateKDEDocDlg::slotDelRecentRadioButtonClicked(){
  doc_button->setEnabled(false);
  doc_edit->setEnabled(false);
  doc_label->setEnabled(false);
}


void CUpdateKDEDocDlg::slotDocButtonClicked(){
  QString name = KFileDialog::getExistingDirectory(doc_edit->text(),this,i18n("New KDE Documentation Directory..."));
  if(!name.isEmpty()){
    doc_edit->setText(name);
  }
}


void CUpdateKDEDocDlg::slotSourceButtonClicked(){
  QString dir = KFileDialog::getExistingDirectory(source_edit->text(),this,i18n("KDE Libs Directory..."));
  if(!dir.isEmpty()){
      source_edit->setText(dir);
  }

}

void CUpdateKDEDocDlg::createShellProcessOutputWidget()
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

  m_pShellProcessOutput->resize(300, 400);

  QObject::connect(m_pShellProcessOutputOKButton, SIGNAL(clicked()), m_pShellProcessOutput, SLOT(accept()));
  QObject::connect(m_proc,SIGNAL(receivedStdout(KProcess*,char*,int)), this, SLOT(slotReceivedStdout(KProcess*,char*,int)) );
  QObject::connect(m_proc,SIGNAL(receivedStderr(KProcess*,char*,int)), this, SLOT(slotReceivedStderr(KProcess*,char*,int)) );
  QObject::connect(m_proc,SIGNAL(processExited(KProcess*)), this, SLOT(slotProcessExited(KProcess*) )) ;
}

void CUpdateKDEDocDlg::slotShowToolProcessOutputDlg()
{
  m_pShellProcessOutputOKButton->setEnabled(false);
  m_pShellProcessOutputLines->clear();
  QApplication::setOverrideCursor(WaitCursor);
  emit newDocCreationStartedNow();
  create_button->setEnabled(false);
  m_pShellProcessOutput->exec();
}

void CUpdateKDEDocDlg::slotReceivedStdout(KProcess*,char* buffer,int count)
{
  QCString test(buffer, count);
  m_pShellProcessOutputLines->insertLine(test);
  m_pShellProcessOutputLines->setCursorPosition(m_pShellProcessOutputLines->numLines(), 0);
}
void CUpdateKDEDocDlg::slotReceivedStderr(KProcess*,char* buffer, int count)
{
  QCString test(buffer, count);
  m_pShellProcessOutputLines->insertLine(test);
  m_pShellProcessOutputLines->setCursorPosition(m_pShellProcessOutputLines->numLines(), 0);
}

void CUpdateKDEDocDlg::slotProcessExited(KProcess*)
{
  m_pShellProcessOutputLines->insertLine("");
  m_pShellProcessOutputLines->insertLine("Finished!");
  m_pShellProcessOutputLines->setCursorPosition(m_pShellProcessOutputLines->numLines(), 0);
  m_pShellProcessOutputOKButton->setEnabled(true);
  create_button->setEnabled(true);
  QApplication::restoreOverrideCursor();

  emit newDocIsCreatedNow(kde_doc_path);
}

void CUpdateKDEDocDlg::setQtDocDir(const QString& qtDocDir)
{
  qt_doc_path = qtDocDir;
}
 
#include "cupdatekdedocdlg.moc"
