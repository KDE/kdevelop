/***************************************************************************
                     cnewfiledlg.cpp - the new file dialog
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
#include "cnewfiledlg.h"
#include "cgeneratenewfile.h"
#include <kmsgbox.h>
#include <iostream.h>
#include <kfiledialog.h>


CNewFileDlg::CNewFileDlg(QWidget* parent,const char* name,bool modal,WFlags f,CProject* p_prj) 
  : QDialog(parent,name,modal,f){
  
  prj = p_prj;
  setCaption("New File...");    
  
  button_group = new QButtonGroup( this, "button_goup" );
  button_group->setGeometry( 290, 150, 200, 110   ); 
  button_group->setFrameStyle( 49 );
  button_group->setTitle( "Projectoptions" );
  button_group->setAlignment( 1 );
  
  check_use_template = new QCheckBox( this, "check_use_template" );
  check_use_template->setGeometry( 300, 110, 100, 30);
  check_use_template->setText( "use Template" );
  check_use_template->setAutoRepeat( FALSE );
  check_use_template->setAutoResize( FALSE );
  
  check_add_to_project = new QCheckBox( this, "check_add_project" );
  check_add_to_project->setGeometry(   300, 170, 100, 30 );
  check_add_to_project->setText( "add to Project" );
  check_add_to_project->setAutoRepeat( FALSE );
  check_add_to_project->setAutoResize( FALSE );
  
  ok = new QPushButton( this, "ok" );
  ok->setGeometry(130, 300, 100, 30 );
  ok->setText( "OK" );
  ok->setAutoRepeat( FALSE );
  ok->setAutoResize( FALSE );
  
  edit = new QLineEdit( this, "edit" );
  edit->setGeometry(   350, 60, 140, 30 );
  edit->setText( "" );
  edit->setMaxLength( 32767 );
  edit->setEchoMode( QLineEdit::Normal );
  edit->setFrame( TRUE );
  
  cancel = new QPushButton( this, "cancel" );
  cancel->setGeometry(300, 300, 100, 30 );
  cancel->setText( "Cancel" );
  cancel->setAutoRepeat( FALSE );
  cancel->setAutoResize( FALSE );
  
  label_filename = new QLabel( this, "label_filename" );
  label_filename->setGeometry(  290, 60, 60, 30 );
  label_filename->setText( "Filename:" );
  label_filename->setAlignment( 289 );
  label_filename->setMargin( -1 );
  
  // the tabview
  tab = new KTabCtl(this);
  tab->setGeometry(20, 33, 260, 230  );
  tab->setBorder(false);
  
  list_gnu = new QListBox( tab, "list_gnu" );
  list_gnu->insertItem( "INSTALL" );
  list_gnu->insertItem( "README" );
  list_gnu->insertItem( "COPYING" );
  list_gnu->insertItem( "AUTHORS" );
  list_gnu->insertItem( "TODO" );
  list_gnu->insertItem( "ChangeLog" );
  list_gnu->setMultiSelection( FALSE );
  list_gnu->setCurrentItem(0);
  
  list_linux = new QListBox( tab, "list_linux" );
  list_linux->insertItem( "lsm-File (.lsm) - for the Linux Software Map" );
  list_linux->setMultiSelection( FALSE );
  list_linux->setCurrentItem(0);
  
  list_kde = new QListBox( tab, "list_kde" );
  list_kde->insertItem( "en_sgml-File (.sgml) - an english user-manual" );
  list_kde->insertItem( "Link-File (.kdelnk) - for the K-Menu" );
  list_kde->setMultiSelection( FALSE );
  list_kde->setCurrentItem(0);
  
  list_cpp = new QListBox( tab, "list_cpp" );
  list_cpp->insertItem( "Header (*.h) - a C/C++ Header File" );
  list_cpp->insertItem( "C/C++ File (*.cpp,*.c,*.cc,*.C) - Impl. File" );
  list_cpp->insertItem( "Empty Textfile" );
  list_cpp->setMultiSelection( FALSE );
  list_cpp->setCurrentItem(0);
  
  tab->addTab(list_cpp,"General");
  tab->addTab(list_gnu,"GNU");
  tab->addTab(list_linux,"Linux");
  tab->addTab(list_kde,"KDE");
  
  
  
  prj_loc_edit = new QLineEdit( this, "prj_loc_edit" );
  prj_loc_edit->setGeometry( 300, 210, 140, 30 );
  prj_loc_edit->setMinimumSize( 0, 0 );
  prj_loc_edit->setMaximumSize( 32767, 32767 );
  prj_loc_edit->setFocusPolicy( QWidget::StrongFocus );
  prj_loc_edit->setBackgroundMode( QWidget::PaletteBase );
  prj_loc_edit->setFontPropagation( QWidget::NoChildren );
  prj_loc_edit->setPalettePropagation( QWidget::NoChildren );
  prj_loc_edit->setText( prj->getProjectDir()+ prj->getSubDir());
  prj_loc_edit->setMaxLength( 32767 );
  prj_loc_edit->setEchoMode( QLineEdit::Normal );
  prj_loc_edit->setFrame( TRUE );
  
  
  loc_button = new QPushButton( this, "loc_button" );
  loc_button->setGeometry( 450, 210, 30, 30 );
  loc_button->setMinimumSize( 0, 0 );
  loc_button->setMaximumSize( 32767, 32767 );
  loc_button->setFocusPolicy( QWidget::TabFocus );
  loc_button->setBackgroundMode( QWidget::PaletteBackground );
  loc_button->setFontPropagation( QWidget::NoChildren );
  loc_button->setPalettePropagation( QWidget::NoChildren );
  loc_button->setText( "..." );
  loc_button->setAutoRepeat( FALSE );
  loc_button->setAutoResize( FALSE );
  
  button_group->insert( check_add_to_project );
  button_group->insert( loc_button );
  
  resize(510,350 );
  setFixedSize(510,350);
  edit->setFocus();
  
  connect(tab,SIGNAL(tabSelected(int)),SLOT(slotTabSelected(int)));
  connect(ok,SIGNAL(clicked()),SLOT(slotOKClicked()));
  connect(cancel,SIGNAL(clicked()),SLOT(reject()));
  connect(loc_button,SIGNAL(clicked()),SLOT(slotLocButtonClicked()));
  connect(check_add_to_project,SIGNAL(clicked()),SLOT(slotAddToProject()));
}

void CNewFileDlg::slotTabSelected(int item){
  if (item == 1){
    edit->setEnabled(false);
  }
  else {
    edit->setEnabled(true);
  }
  current = item;
}
void CNewFileDlg::slotOKClicked(){
  QString text = edit->text();
  if ( (fileType() == "CPP") && 
       !(text.right(4) == ".cpp" || text.right(3) == ".cc" 
	|| text.right(2) == ".C" || text.right(2) == ".c")){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .cpp,.c,.cc or .C !")
		     ,KMsgBox::EXCLAMATION);
    return;
  }
  if ( (fileType() == "HEADER") && (text.right(2) != ".h")){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .h !"),KMsgBox::EXCLAMATION);
    return;
  }
  if ( (fileType() == "LSM") && (text.right(4) != ".lsm")){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .lsm !"),KMsgBox::EXCLAMATION);
    return;
   }
  if ( (fileType() == "KDELNK") && (text.right(7) != ".kdelnk")){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .kdelnk !"),KMsgBox::EXCLAMATION);
    return;
  }
  if ( (fileType() == "SGML") && (text.right(5) != ".sgml")){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .sgml !"),KMsgBox::EXCLAMATION);
    return;
  }
  if (text.isEmpty() && current != 1){
    KMsgBox::message(this,i18n("Error..."),i18n("You must enter a filename!"),KMsgBox::EXCLAMATION);
    return;
  }
  if(addToProject() == true && (location().contains(prj->getProjectDir())) == 0 ){
    KMsgBox::message(this,i18n("Error..."),i18n("You must choose a location,that is in your project-dir!")
		     ,KMsgBox::EXCLAMATION);
    return;
  }

   /*************----------generate the new File----------****************/
   QString filename = fileName();
   QString section;
   QString type;
   QString filetype = fileType();
   QString complete_filename;
   CGenerateNewFile generator;
  

   // contruct the complete_filename
   complete_filename = location() + filename;
   
   // check if generate a empty file or generate one
   if (useTemplate() && (filetype != "TEXTFILE")){ // generate one,textfile always empty
    
     if (filetype == "HEADER"){
      generator.genHeaderFile(complete_filename,prj);
      type = "HEADER";
     }
     if (filetype == "CPP"){
       generator.genCPPFile(complete_filename,prj);
       type = "SOURCE";
    }
     if (filetype == "LSM"){
       generator.genLSMFile(complete_filename,prj);
       type = "DATA";
     }
     if (filetype == "KDELNK"){
       generator.genKDELnkFile(complete_filename,prj);
       type = "DATA";
     }
     if (filetype == "EN_SGML"){
       generator.genEngHandbook(complete_filename,prj);
       type = "DATA";
     }
     if (filetype == "GNU"){ 
       generator.genGNUFile(complete_filename,prj);
       type = "DATA";
     }
   }
   else { // no template, -> empty file
     QFile file(complete_filename);
     file.open(IO_ReadWrite);
     file.close();
   }
   accept();
}
QString CNewFileDlg::fileName(){
  QString str;
  if (current == 1){ // GNU
    str = list_gnu->text(list_gnu->currentItem());
    return str;
  }
  return edit->text();
}
QString CNewFileDlg::fileType(){ 
  QString str;
  if (current == 0){ // cpp/header
    str = list_cpp->text(list_cpp->currentItem());
    if (str == "C/C++ File (*.cpp,*.c,*.cc,*.C) - Impl. File"){
      return "CPP";
    }
    if (str == "Header (*.h) - a C/C++ Header File"){
      return "HEADER";
    }
    if (str == "Empty Textfile"){
      return "TEXTFILE";
    }
  }
  if (current == 1){ // GNU
    return "GNU";
  }
  
  if (current == 2){ // linux
    str = list_linux->text(list_linux->currentItem());
    if (str == "lsm-File (.lsm) - for the Linux Software Map"){
      return "LSM";
    }
  }
  if (current == 3){ // kde
    str = list_kde->text(list_kde->currentItem());
    if (str == "Link-File (.kdelnk) - for the K-Menu"){
      return "KDELNK";
    }
    if (str == "en_sgml-File (.sgml) - an english user-manual"){
      return "EN_SGML";
    }
  }
  return "TEST";
}
bool CNewFileDlg::useTemplate(){
  return check_use_template->isChecked();
}
bool CNewFileDlg::addToProject(){
  return check_add_to_project->isChecked();
}
void CNewFileDlg::setUseTemplate(){
  check_use_template->setChecked(true);
}
void CNewFileDlg::setAddToProject(){
  check_add_to_project->setChecked(true);
}
void CNewFileDlg::slotLocButtonClicked(){
  QString str=  KDirDialog::getDirectory(prj_loc_edit->text(),this,"test");
  if(!str.isEmpty()){
    prj_loc_edit->setText(str);
  }
}
QString CNewFileDlg::location(){
  return prj_loc_edit->text();
}
void CNewFileDlg::slotAddToProject(){
  if(check_add_to_project->isChecked()){
    prj_loc_edit->setEnabled(true);
  }
  else{
    prj_loc_edit->setEnabled(false);
  }
}
