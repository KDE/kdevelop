/***************************************************************************
                     kdevsetup.cpp - the setup dialog for KDevelop
                             -------------------                                         

    version              :                                   
    begin                : 17 Aug 1998                                        
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

#include "ckdevsetupdlg.h"


// SETUP DIALOG
CKDevSetupDlg::CKDevSetupDlg( QWidget *parent, const char *name )
    : QTabDialog( parent, name,TRUE )
{
  setFixedSize(462,218);
  

  setCaption( i18n("KDevelop Setup" ));
  KApplication *app=KApplication::getKApplication();
  config=app->getConfig();
  
  
  // ****************** the Doc_LOcation_DLG ********************
  QWidget *w = new QWidget( this, "page one" );    
  config->setGroup(i18n("Doc_Location"));


  KQuickHelp::add(w, i18n("Enter the path to your QT and KDE-Libs\n"
		          "Documentation for the Documentation Browser.\n"
                          "QT usually comes with complete Documentation\n"
			  "whereas for KDE you can create the Documentation\n"
			  "easiely by switching to menuentry"
			  "<i>Documentation -> \n Update KDE-Documentation</i>."
			  "Please use the Update dialog whenever you update \n"
			  "your KDE Libs."));

  qt_edit = new QLineEdit( w, "qt_edit" );
  qt_edit->setGeometry( 140, 30, 220, 30 );
  qt_edit->setText( config->readEntry("doc_qt"));
  qt_edit->setMaxLength( 32767 );
  
  QPushButton* qt_button;
  qt_button = new QPushButton( w, "qt_button" );
  qt_button->setGeometry( 390, 30, 30, 30 );
  qt_button->setText( "..." );
  connect(qt_button,SIGNAL(clicked()),SLOT(slotQtClicked()));
  
  QLabel* qt_label;
  qt_label = new QLabel( w, "qt_label" );
  qt_label->setGeometry( 10, 30, 90, 30 );
  qt_label->setText( i18n("Qt-Library Doc:") );
  

  KQuickHelp::add(qt_edit,
  KQuickHelp::add(qt_button,
  KQuickHelp::add(qt_label,
		i18n("Enter the path to your QT-Documentation\n"
		     "here. To access the path easier please\n"
		     "press the pushbutton on the right to change\n"
		     "directories.\n\nUsually the QT-Documentation is\n"
		     "located in <i><blue>$QTDIR/html</i>"))));	
  
  kde_edit = new QLineEdit( w, "kde_edit");
  kde_edit->setGeometry( 140, 80, 220, 30 );
  kde_edit->setText(config->readEntry("doc_kde"));
  kde_edit->setMaxLength( 32767 );
  kde_edit->setEchoMode( QLineEdit::Normal );
  kde_edit->setFrame( TRUE );
  
  QPushButton* kde_button;
  kde_button = new QPushButton( w, "kde_button" );
  kde_button->setGeometry( 390, 80, 30, 30 );
  kde_button->setText( "..." );
  connect(kde_button,SIGNAL(clicked()),SLOT(slotKDEClicked()));
  
  QLabel* kde_label;
  kde_label = new QLabel( w, "kde_label" );
  kde_label->setGeometry( 10, 80, 130, 30 );
  kde_label->setText( i18n("KDE Library Doc:") );

  KQuickHelp::add(kde_edit,
  KQuickHelp::add(kde_button,
  KQuickHelp::add(kde_label,
		i18n("Enter the path to your KDE-Documentation\n"
		     "here. To access the path easier please\n"
		     "press the pushbutton on the right to change\n"
		     "directories.\n\n"
		     "If you have no kdelibs Documentation installed,\n"
		     "switch to menuentry <blue><i>Documentation ->\n"
		     "Update KDE-Documentation</i><black> to create it."))));  

      
  addTab( w, i18n("Doc-Location" ));
  

  
    // **************set the button*********************
  setOkButton(i18n("OK"));
  setCancelButton(i18n("Cancel"));
  connect( this, SIGNAL(applyButtonPressed()), SLOT(ok()) );
  
}



  

void CKDevSetupDlg::ok(){
  
  QString text;
  config->setGroup(i18n("Doc_Location"));
  text = qt_edit->text();
  config->writeEntry("doc_qt",text);
  text = kde_edit->text();
  config->writeEntry("doc_kde" , text);
  
  config->sync();
  close();
}

void CKDevSetupDlg::slotQtClicked(){
  QString dir;
  dir = KFileDialog::getDirectory(config->readEntry("doc_qt"));
  if (!dir.isEmpty()){
  qt_edit->setText(dir);
  config->setGroup(i18n("Doc_Location"));
  config->writeEntry("doc_qt",dir);
  }
}
void CKDevSetupDlg::slotKDEClicked(){
  QString dir;
  dir = KFileDialog::getDirectory(config->readEntry("doc_kde"));
  if (!dir.isEmpty()){
    kde_edit->setText(dir);
    config->setGroup(i18n("Doc_Location"));
    config->writeEntry("doc_kde",dir);
  }
}
