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

#include <qdir.h>

#include <kapp.h>
#include <kfiledialog.h>
#include <kmsgbox.h>
#include <klocale.h>
#include <qprogressdialog.h>
#include "cupdatekdedocdlg.h"


CUpdateKDEDocDlg::CUpdateKDEDocDlg(QWidget *parent, const char *name,KShellProcess* proc,KConfig* config) : QDialog(parent,name,true) {
    
    conf = config;
    config->setGroup("Doc_Location");
    this->doc_path = config->readEntry("doc_kde", KDELIBS_DOCDIR);
    this->proc = proc;
    
    setCaption(i18n("KDE Library Documentation Update..."));

    install_box = new QButtonGroup( this, "install_box" );
    install_box->setGeometry( 20, 80, 490, 170 );
    install_box->setMinimumSize( 0, 0 );
    install_box->setMaximumSize( 32767, 32767 );
    install_box->setFocusPolicy( QWidget::NoFocus );
    install_box->setBackgroundMode( QWidget::PaletteBackground );
    install_box->setFontPropagation( QWidget::NoChildren );
    install_box->setPalettePropagation( QWidget::NoChildren );
    install_box->setFrameStyle( 49 );
    install_box->setTitle(i18n("Choose installation mode:") );
    install_box->setAlignment( 1 );
    
    source_label = new QLabel( this, "source_label" );
    source_label->setGeometry( 30, 30, 210, 30 );
    source_label->setMinimumSize( 0, 0 );
    source_label->setMaximumSize( 32767, 32767 );
    source_label->setFocusPolicy( QWidget::NoFocus );
    source_label->setBackgroundMode( QWidget::PaletteBackground );
    source_label->setFontPropagation( QWidget::NoChildren );
    source_label->setPalettePropagation( QWidget::NoChildren );
    source_label->setText(i18n("new KDE Libs sources path:") );
    source_label->setAlignment( 289 );
    source_label->setMargin( -1 );

    source_edit = new QLineEdit( this, "source_edit" );
    source_edit->setGeometry( 240, 30, 230, 30 );
    source_edit->setMinimumSize( 0, 0 );
    source_edit->setMaximumSize( 32767, 32767 );
    source_edit->setFocusPolicy( QWidget::StrongFocus );
    source_edit->setBackgroundMode( QWidget::PaletteBase );
    source_edit->setFontPropagation( QWidget::NoChildren );
    source_edit->setPalettePropagation( QWidget::NoChildren );
    source_edit->setText( QDir::homeDirPath() );
    source_edit->setMaxLength( 32767 );
    source_edit->setEchoMode( QLineEdit::Normal );
    source_edit->setFrame( TRUE );
    
    source_button = new QPushButton( this, "source_button" );
    source_button->setGeometry( 480, 30, 30, 30 );
    source_button->setMinimumSize( 0, 0 );
    source_button->setMaximumSize( 32767, 32767 );
    source_button->setFocusPolicy( QWidget::TabFocus );
    source_button->setBackgroundMode( QWidget::PaletteBackground );
    source_button->setFontPropagation( QWidget::NoChildren );
    source_button->setPalettePropagation( QWidget::NoChildren );
		QPixmap pix;
  	pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
    source_button->setPixmap(pix);
    source_button->setAutoRepeat( FALSE );
    source_button->setAutoResize( FALSE );

    del_recent_radio_button = new QRadioButton( this, "del_recent_radio_button" );
    del_recent_radio_button->setGeometry( 40, 110, 450, 30 );
    del_recent_radio_button->setMinimumSize( 0, 0 );
    del_recent_radio_button->setMaximumSize( 32767, 32767 );
    del_recent_radio_button->setFocusPolicy( QWidget::TabFocus );
    del_recent_radio_button->setBackgroundMode( QWidget::PaletteBackground );
    del_recent_radio_button->setFontPropagation( QWidget::NoChildren );
    del_recent_radio_button->setPalettePropagation( QWidget::NoChildren );
    del_recent_radio_button->setText(i18n("Delete old Documentation and install to recent Documentation path"));
    del_recent_radio_button->setAutoRepeat( FALSE );
    del_recent_radio_button->setAutoResize( FALSE );
    del_recent_radio_button->setChecked( TRUE );
    KQuickHelp::add(del_recent_radio_button,
		    i18n("Checking this will delete the current documentation\n"
			 "and replacing it with the new generated documentation\n" 
			 "in the same path.")); 
    
    
    del_new_radio_button = new QRadioButton( this, "del_new_radio_button" );
    del_new_radio_button->setGeometry( 40, 150, 440, 30 );
    del_new_radio_button->setMinimumSize( 0, 0 );
    del_new_radio_button->setMaximumSize( 32767, 32767 );
    del_new_radio_button->setFocusPolicy( QWidget::TabFocus );
    del_new_radio_button->setBackgroundMode( QWidget::PaletteBackground );
    del_new_radio_button->setFontPropagation( QWidget::NoChildren );
    del_new_radio_button->setPalettePropagation( QWidget::NoChildren );
    del_new_radio_button->setText(i18n("Delete old Documentation and install to new Documentation path") );
    del_new_radio_button->setAutoRepeat( FALSE );
    del_new_radio_button->setAutoResize( FALSE );
    KQuickHelp::add(del_new_radio_button,
		    i18n("Checking this will delete the current documentation\n"
			 "and lets you choose a path in the input field below\n"
			 "where the new generated documentation will be"
			 "installed."));  
    
    leave_new_radio_button = new QRadioButton( this, "leave_new_radio_button" );
    leave_new_radio_button->setGeometry( 40, 190, 450, 30 );
    leave_new_radio_button->setMinimumSize( 0, 0 );
    leave_new_radio_button->setMaximumSize( 32767, 32767 );
    leave_new_radio_button->setFocusPolicy( QWidget::TabFocus );
    leave_new_radio_button->setBackgroundMode( QWidget::PaletteBackground );
    leave_new_radio_button->setFontPropagation( QWidget::NoChildren );
    leave_new_radio_button->setPalettePropagation( QWidget::NoChildren );
    leave_new_radio_button->setText(i18n("Leave old Documention untouched and install to new Documention path") );
    leave_new_radio_button->setAutoRepeat( FALSE );
    leave_new_radio_button->setAutoResize( FALSE );
    KQuickHelp::add(leave_new_radio_button,
		    i18n("This doesn't delete your current documentation and leaves it\n"
			 "is now and you can select a new path for the new kdelibs\n"
			 "documentation. CAUTION: Don't insert the same path where\n"
			 "your recent documentation is installed- this may mess up\n"
			 "the documentation by mixing old and new files!"));
    
    doc_label = new QLabel( this, "doc_label" );
    doc_label->setGeometry( 30, 270, 210, 30 );
    doc_label->setMinimumSize( 0, 0 );
    doc_label->setMaximumSize( 32767, 32767 );
    doc_label->setFocusPolicy( QWidget::NoFocus );
    doc_label->setBackgroundMode( QWidget::PaletteBackground );
    doc_label->setFontPropagation( QWidget::NoChildren );
    doc_label->setPalettePropagation( QWidget::NoChildren );
    doc_label->setText(i18n("new KDE Libs Documentation path:") );
    doc_label->setAlignment( 289 );
    doc_label->setMargin( -1 );
    doc_label->setEnabled(false);

    doc_edit = new QLineEdit( this, "doc_edit" );
    doc_edit->setGeometry( 240, 270, 230, 30 );
    doc_edit->setMinimumSize( 0, 0 );
    doc_edit->setMaximumSize( 32767, 32767 );
    doc_edit->setFocusPolicy( QWidget::StrongFocus );
    doc_edit->setBackgroundMode( QWidget::PaletteBase );
    doc_edit->setFontPropagation( QWidget::NoChildren );
    doc_edit->setPalettePropagation( QWidget::NoChildren );
    doc_edit->setText(doc_path);
    doc_edit->setMaxLength( 32767 );
    doc_edit->setEchoMode( QLineEdit::Normal );
    doc_edit->setFrame( TRUE );
    doc_edit->setEnabled(false);

    doc_button = new QPushButton( this, "doc_button" );
    doc_button->setGeometry( 480, 270, 30, 30 );
    doc_button->setMinimumSize( 0, 0 );
    doc_button->setMaximumSize( 32767, 32767 );
    doc_button->setFocusPolicy( QWidget::TabFocus );
    doc_button->setBackgroundMode( QWidget::PaletteBackground );
    doc_button->setFontPropagation( QWidget::NoChildren );
    doc_button->setPalettePropagation( QWidget::NoChildren );
    doc_button->setPixmap(pix);
    doc_button->setAutoRepeat( FALSE );
    doc_button->setAutoResize( FALSE );
    doc_button->setEnabled(false);

    install_box->insert( del_recent_radio_button );
    install_box->insert( del_new_radio_button );
    install_box->insert( leave_new_radio_button );
    
    
    KQuickHelp::add(source_label,
		    KQuickHelp::add(source_edit,
				    KQuickHelp::add(source_button, i18n("Insert the path to the current\n"
		       						"KDE-Libs sourcecodes here. This is\n"
		       						"where you have unpacked e.g. a kdelibs\n"
		       						"snapshot a la /snapshot/kdelibs."))));
    
    KQuickHelp::add(doc_label,
		KQuickHelp::add(doc_edit,
	  KQuickHelp::add(doc_button, i18n("Insert the path where you want to have\n"
				   "the new generated documentation installed\n"
				   "Note: the path information in Setup will\n"
               			   "be updated automatically, you don't have\n"
				   "to change them to the new doc path."))));

    ok_button = new QPushButton( this, "ok_button" );
    ok_button->setGeometry( 140, 320, 100, 30 );
    ok_button->setMinimumSize( 0, 0 );
    ok_button->setMaximumSize( 32767, 32767 );
    ok_button->setFocusPolicy( QWidget::TabFocus );
    ok_button->setBackgroundMode( QWidget::PaletteBackground );
    ok_button->setFontPropagation( QWidget::NoChildren );
    ok_button->setPalettePropagation( QWidget::NoChildren );
    ok_button->setText( i18n("OK") );
    ok_button->setAutoRepeat( FALSE );
    ok_button->setAutoResize( FALSE );
    ok_button->setDefault( true );

    cancel_button = new QPushButton( this, "cancel_button" );
    cancel_button->setGeometry( 270, 320, 100, 30 );
    cancel_button->setMinimumSize( 0, 0 );
    cancel_button->setMaximumSize( 32767, 32767 );
    cancel_button->setFocusPolicy( QWidget::TabFocus );
    cancel_button->setBackgroundMode( QWidget::PaletteBackground );
    cancel_button->setFontPropagation( QWidget::NoChildren );
    cancel_button->setPalettePropagation( QWidget::NoChildren );
    cancel_button->setText(i18n("Cancel"));
    cancel_button->setAutoRepeat( FALSE );
    cancel_button->setAutoResize( FALSE );


  bUpdated=false;

  resize( 530,380 );
  setMinimumSize( 0, 0 );
  setMaximumSize( 32767, 32767 );
  
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
  connect(ok_button,SIGNAL(clicked()),SLOT(OK()));
  connect(leave_new_radio_button,SIGNAL(clicked()),SLOT(slotLeaveNewRadioButtonClicked()));
  connect(del_new_radio_button,SIGNAL(clicked()),SLOT(slotDelNewRadioButtonClicked()));
  connect(del_recent_radio_button,SIGNAL(clicked()),SLOT(slotDelRecentRadioButtonClicked()));

  connect(doc_button,SIGNAL(clicked()),SLOT(slotDocButtonClicked()));
  connect(source_button,SIGNAL(clicked()),SLOT(slotSourceButtonClicked()));

}


CUpdateKDEDocDlg::~CUpdateKDEDocDlg(){
}


void CUpdateKDEDocDlg::OK(){
    KShellProcess proc_rm;
    QString kdelibs_path = source_edit->text();
  
  if(kdelibs_path.right(1) != "/"){
    kdelibs_path = kdelibs_path +"/";
  }
  // check if path (TO GENERATE the doc from the sources) is set correctly
  QString kde_testfile=kdelibs_path+"kdecore/kapp.h"; // test if the path really is the kdelibs path
  if(!QFileInfo(kde_testfile).exists()){
    KMsgBox::message(this,i18n("The selected path is not correct!"),i18n("The chosen path for the KDE-Libs does not\n"
									 "lead to the KDE Libraries. Please choose the\n"
									 "correct path.This is where you have unpacked\n"
									 "e.g. a kdelibs snapshot a la /snapshot/kdelibs."),KMsgBox::EXCLAMATION);
    return;
  }
  
  QString new_doc_path = doc_path;
  if(!del_recent_radio_button->isChecked()){ // not recent doc path
    new_doc_path = doc_edit->text();
    conf->setGroup("Doc_Location");
    conf->writeEntry("doc_kde",new_doc_path);
  }

  if(new_doc_path.right(1) != "/"){
    new_doc_path += "/";
  }

  if(doc_path.right(1) != "/"){
    doc_path += "/";
  }
  QDir().mkdir(new_doc_path);
  if(!QFileInfo(new_doc_path).isWritable()){
    KMsgBox::message(this,i18n("Error in creating documentation!"),
                         i18n("You need write permission to create\n"
			 "the documentation in\n")+new_doc_path, KMsgBox::STOP);
    return;
  }

  if(!leave_new_radio_button->isChecked()){ // ok,let's delete it,attentation!!!
      proc_rm.clearArguments();
      if(!QFileInfo(doc_path).exists())
      {
         KMsgBox::message(this,i18n("Old documentation deletion!"),
                i18n("The old documentation path\n")+
                doc_path+
                i18n("\ndoesn´t exist anymore.")+
                i18n("\nProcess will continue without deletion...")
              );
      }
      else
      {
        if(!QFileInfo(doc_path).isWritable())
        {
         KMsgBox::message(this,i18n("Old documentation deletion!"),
                i18n("You have no write permission to delete\n"
                "the old documentation in\n")+doc_path+
                i18n("\nProcess will continue without deletion...")
              );
        }
        else
        {
          if(QDir::setCurrent(doc_path)){
            QString command;
// protect the rest of the files in the directory...
//   maybe someone installs the htmls in the source dir of the
//   kdelib
	    command= "rm -f -r kdoc-reference/;rm -f -r kdecore/*.htm*;"
                  "rm -f -r kdeui/*.htm*;rm -f -r kio/*.htm*;"
		  "rm -f -r kimgio/*.htm*;rm -f -r mediatool/*.htm*;"
                  "rm -f -r kdeutils/*.htm*;"
                  "rm -f -r jscript/*.htm*;rm -f -r kfile/*.htm*;"
                  "rm -f -r khtmlw/*.htm*;rm -f -r kfmlib/*.htm*;"
                  "rm -f -r kab/*.htm*;rm -f -r kspell/*.htm*;"
// use here rmdir (so the directory won't be deleted if there are other
//  files than the documentation
                  "rmdir kdecore; rmdir kdeui;rmdir kio;"
                  "rmdir kimgio; rmdir mediatool; rmdir kdeutils;"
                  "rmdir jscript; rmdir kfile; rmdir khtmlw; rmdir kfmlib;"
                  "rmdir kab; rmdir kspell";
            //  if the old path and the new doc path differs then
            //  delete the old doc dir
            if (doc_path!=new_doc_path)
               command += "; cd ~; rmdir "+doc_path;

            proc_rm << command;
	    proc_rm.start(KShellProcess::Block,KShellProcess::AllOutput);
           }
        }
      }
    }

  proc->clearArguments();
  QDir::setCurrent(kdelibs_path);

  
  conf->setGroup("Doc_Location");
  QString qtPath=conf->readEntry("doc_qt", QT_DOCDIR);
  bool qt_test=false;
  QString qt_testfile=qtPath+"classes.html";
  if(QFileInfo(qt_testfile).exists()){
    qt_test=true;
  }

#if WITH_KDOC2
  QString cmd;
  if(! qt_test)
      {
          int qt_set=KMsgBox::yesNo(this,i18n("Question"),i18n("The Qt-Documentation path is not set correctly.\n"
                                                               "If you want your KDE-library documentation to\n"
                                                               "be cross-referenced to the Qt-library, you have\n"
                                                               "to set the correct path to your Qt-library\n"
                                                               "documentation first.\n"
                                                               "Do you want to set the Qt-Documentation path first ?"),KMsgBox::QUESTION);
          if(qt_set==1)
              return;
      }
  else
      {
          cmd = "qt2kdoc --url=";
          cmd += qtPath;
          cmd += " --outdir=";
          cmd += new_doc_path;
          cmd += " --compress ";
          cmd += qtPath;
          cmd += ";\n";
      }

  cmd += "makekdedoc --libdir=";
  cmd += new_doc_path;
  cmd += " --outputdir=";
  cmd += new_doc_path;
  *proc << cmd;
  
#else
  
  if(!qt_test){ // don't cross-reference to qt
    int qt_set=KMsgBox::yesNo(this,i18n("Question"),i18n("The Qt-Documentation path is not set correctly.\n"
                                                    "If you want your KDE-library documentation to\n"
                                                    "be cross-referenced to the Qt-library, you have\n"
                                                    "to set the correct path to your Qt-library\n"
                                                    "documentation first.\n"
                                                    "Do you want to set the Qt-Documentation path first ?"),KMsgBox::QUESTION);
    if(qt_set==1)
      return;  // exit the update dialog
    else{  // don't return to the setup to set the qt-path and index without qt
      *proc << "mkdir "+new_doc_path+"kdoc-reference;"
	       "cd kdecore;kdoc -d "+ new_doc_path + "kdecore -ufile:" + new_doc_path + "kdecore/ -L"
               + new_doc_path +"kdoc-reference kdecore *.h;"
               "cd ../kdeui;kdoc -d " + new_doc_path + "kdeui -ufile:" + new_doc_path + "kdeui/ -L"
               + new_doc_path +"kdoc-reference kdeui *.h -lkdecore;"
               "cd ../kio;kdoc -d "+ new_doc_path + "kio -ufile:" + new_doc_path + "kio/ -L"
               + new_doc_path +"kdoc-reference kio *.h -lkdecore -lkdeui;"
               "cd ../kimgio;kdoc -d "+ new_doc_path + "kimgio -ufile:" + new_doc_path + "kimio/ -L"
               + new_doc_path +"kdoc-reference kimgio *.h -lkdecore -lkdeui -lkio;"
               "cd ../mediatool;kdoc -d "+ new_doc_path + "mediatool -ufile:" + new_doc_path + "mediatool/ -L"
               + new_doc_path +"kdoc-reference mediatool *.h -lkdecore -lkdeui -lkio;"
               "cd ../kdeutils;kdoc -d "+ new_doc_path + "kdeutils -ufile:" + new_doc_path + "kdeutils/ -L"
               + new_doc_path +"kdoc-reference kdeutils *.h -lkdecore -lkdeui;"
               "cd ../jscript;kdoc -d "+ new_doc_path + "jscript -ufile:" + new_doc_path + "jscript/ -L"
               + new_doc_path +"kdoc-reference jscript *.h -lkdecore -lkdeui;"
               "cd ../khtmlw;kdoc -d "+ new_doc_path + "khtmlw -ufile:" + new_doc_path + "khtmlw/ -L"
               + new_doc_path +"kdoc-reference khtmlw *.h -lkdecore -lkdeui -ljscript;"
               "cd ../kfile;kdoc -d "+ new_doc_path + "kfile -ufile:" + new_doc_path + "kfile/ -L"
               + new_doc_path +"kdoc-reference kfile *.h -lkdecore -lkdeui;"
               "cd ../kfmlib;kdoc -d "+ new_doc_path + "kfmlib -ufile:" + new_doc_path + "kfmlib/ -L"
               + new_doc_path +"kdoc-reference kfmlib *.h -lkdecore -lkdeui;"
               "cd ../kab;kdoc -d "+ new_doc_path + "kab -ufile:" + new_doc_path + "kab/ -L"
               + new_doc_path +"kdoc-reference kab *.h -lkdecore -lkdeui;"
               "cd ../kspell;kdoc -d "+ new_doc_path + "kspell -ufile:" + new_doc_path + "kspell/ -L"
               + new_doc_path +"kdoc-reference kspell *.h -lkdecore -lkdeui";
    }
  }
  else{  // cross-reference to qt
    *proc << "mkdir "+new_doc_path+"kdoc-reference; qt2kdoc -ufile:" + qtPath + " -o" + new_doc_path +
               "kdoc-reference " + qtPath + "classes.html;"
               "cd kdecore;kdoc -d "+ new_doc_path + "kdecore -ufile:" + new_doc_path + "kdecore/ -L"
               + new_doc_path +"kdoc-reference kdecore *.h -lqt;"
               "cd ../kdeui;kdoc -d " + new_doc_path + "kdeui -ufile:" + new_doc_path + "kdeui/ -L"
               + new_doc_path +"kdoc-reference kdeui *.h -lqt -lkdecore;"
               "cd ../kio;kdoc -d "+ new_doc_path + "kio -ufile:" + new_doc_path + "kio/ -L"
               + new_doc_path +"kdoc-reference kio *.h -lqt -lkdecore -lkdeui;"
               "cd ../kimgio;kdoc -d "+ new_doc_path + "kimgio -ufile:" + new_doc_path + "kimio/ -L"
               + new_doc_path +"kdoc-reference kimgio *.h -lqt -lkdecore -lkdeui -lkio;"
               "cd ../mediatool;kdoc -d "+ new_doc_path + "mediatool -ufile:" + new_doc_path + "mediatool/ -L"
               + new_doc_path +"kdoc-reference mediatool *.h -lqt -lkdecore -lkdeui -lkio;"
               "cd ../kdeutils;kdoc -d "+ new_doc_path + "kdeutils -ufile:" + new_doc_path + "kdeutils/ -L"
               + new_doc_path +"kdoc-reference kdeutils *.h -lqt -lkdecore -lkdeui;"
               "cd ../jscript;kdoc -d "+ new_doc_path + "jscript -ufile:" + new_doc_path + "jscript/ -L"
               + new_doc_path +"kdoc-reference jscript *.h -lqt -lkdecore -lkdeui;"
               "cd ../khtmlw;kdoc -d "+ new_doc_path + "khtmlw -ufile:" + new_doc_path + "khtmlw/ -L"
               + new_doc_path +"kdoc-reference khtmlw *.h -lqt -lkdecore -lkdeui -ljscript;"
               "cd ../kfile;kdoc -d "+ new_doc_path + "kfile -ufile:" + new_doc_path + "kfile/ -L"
               + new_doc_path +"kdoc-reference kfile *.h -lqt -lkdecore -lkdeui;"
               "cd ../kfmlib;kdoc -d "+ new_doc_path + "kfmlib -ufile:" + new_doc_path + "kfmlib/ -L"
               + new_doc_path +"kdoc-reference kfmlib *.h -lqt -lkdecore -lkdeui;"
               "cd ../kab;kdoc -d "+ new_doc_path + "kab -ufile:" + new_doc_path + "kab/ -L"
               + new_doc_path +"kdoc-reference kab *.h -lqt -lkdecore -lkdeui;"
               "cd ../kspell;kdoc -d "+ new_doc_path + "kspell -ufile:" + new_doc_path + "kspell/ -L"
               + new_doc_path +"kdoc-reference kspell *.h -lqt -lkdecore -lkdeui";
  } // end cross-reference qt

#endif
  
  proc->start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  bUpdated=true;
  doc_path=new_doc_path; // all went ok... so set the new doc_path
  accept();
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
  QString name = KDirDialog::getDirectory(doc_edit->text(),this,i18n("New KDE Documentation Directory..."));
  if(!name.isEmpty()){
    doc_edit->setText(name);
  }
}


void CUpdateKDEDocDlg::slotSourceButtonClicked(){
  QString dir = KDirDialog::getDirectory(source_edit->text(),this,i18n("KDE Libs Directory..."));
  if(!dir.isEmpty()){
      source_edit->setText(dir);
  }

}

