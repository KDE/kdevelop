/***************************************************************************
                          cfilepropdlg.cpp  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Oct 17 1998                                           
    copyright            : (C) 1998,1999 by Sandy Meier                         
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


#include "cfilepropdlg.h"
#include <iostream.h>
#include <qfileinfo.h>
#include "debug.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/ 
CFilePropDlg::CFilePropDlg(QWidget *parent, const char *name,CProject* prj,QString preselecteditem ) : QDialog(parent,name,true) {
  this->prj = prj; // save the pointer
  setCaption(i18n("File Properties"));
 
  distribution_group = new QButtonGroup( this, "distribution_group" );
  distribution_group->setGeometry( 340, 140, 310, 60 );
  distribution_group->setMinimumSize( 0, 0 );
  distribution_group->setMaximumSize( 32767, 32767 );
  distribution_group->setFocusPolicy( QWidget::NoFocus );
  distribution_group->setBackgroundMode( QWidget::PaletteBackground );
  distribution_group->setFontPropagation( QWidget::NoChildren );
  distribution_group->setPalettePropagation( QWidget::NoChildren );
  distribution_group->setFrameStyle( 49 );
  distribution_group->setTitle(i18n("Distribution") );
  distribution_group->setAlignment( 1 );
  
  type_group = new QButtonGroup( this, "type_group" );
  type_group->setGeometry( 530, 20, 120, 110 );
  type_group->setMinimumSize( 0, 0 );
  type_group->setMaximumSize( 32767, 32767 );
  type_group->setFocusPolicy( QWidget::NoFocus );
  type_group->setBackgroundMode( QWidget::PaletteBackground );
  type_group->setFontPropagation( QWidget::NoChildren );
  type_group->setPalettePropagation( QWidget::NoChildren );
  type_group->setFrameStyle( 49 );
  type_group->setTitle(i18n("Filetype") );
  type_group->setAlignment( 1 );
  
  installion_group = new QButtonGroup( this, "installion_group" );
  installion_group->setGeometry( 340, 220, 310, 120 );
  installion_group->setMinimumSize( 0, 0 );
  installion_group->setMaximumSize( 32767, 32767 );
  installion_group->setFocusPolicy( QWidget::NoFocus );
  installion_group->setBackgroundMode( QWidget::PaletteBackground );
  installion_group->setFontPropagation( QWidget::NoChildren );
  installion_group->setPalettePropagation( QWidget::NoChildren );
  installion_group->setFrameStyle( 49 );
  installion_group->setTitle(i18n("Installation") );
  installion_group->setAlignment( 1 );
  
  file_group = new QButtonGroup( this, "file_group" );
  file_group->setGeometry( 340, 20, 180, 110 );
  file_group->setMinimumSize( 0, 0 );
  file_group->setMaximumSize( 32767, 32767 );
  file_group->setFocusPolicy( QWidget::NoFocus );
  file_group->setBackgroundMode( QWidget::PaletteBackground );
  file_group->setFontPropagation( QWidget::NoChildren );
  file_group->setPalettePropagation( QWidget::NoChildren );
  file_group->setFrameStyle( 49 );
  file_group->setTitle(i18n("File") );
  file_group->setAlignment( 1 );

  
  log_tree = new CLogFileView( this, "log_tree" );
  log_tree->setGeometry( 20, 20, 300, 320 );
  if(preselecteditem != ""){
    log_tree->setPreSelectedItem(preselecteditem);
  }
  else{
    log_tree->setFirstItemSelected(); // select the first item
      }
  log_tree->setAllGroupsOpened();
  log_tree->refresh(prj);
  
  type_combo = new QComboBox( FALSE, this, "type_combo" );
  type_combo->setGeometry( 540, 60, 100, 30 );
  type_combo->setMinimumSize( 0, 0 );
  type_combo->setMaximumSize( 32767, 32767 );
  type_combo->setFocusPolicy( QWidget::StrongFocus );
  type_combo->setBackgroundMode( QWidget::PaletteBackground );
  type_combo->setFontPropagation( QWidget::NoChildren );
  type_combo->setPalettePropagation( QWidget::NoChildren );
  type_combo->setSizeLimit( 10 );
  type_combo->setAutoResize( FALSE );
  type_combo->insertItem( "HEADER" );
  type_combo->insertItem( "SOURCE" );
  type_combo->insertItem( "SCRIPT" );
  type_combo->insertItem( "DATA" );
  type_combo->insertItem( "PO" );
  
  
  install_loc_edit = new QLineEdit( this, "install_loc_edit" );
  install_loc_edit->setGeometry( 350, 300, 290, 30 );
  install_loc_edit->setMinimumSize( 0, 0 );
  install_loc_edit->setMaximumSize( 32767, 32767 );
  install_loc_edit->setFocusPolicy( QWidget::StrongFocus );
  install_loc_edit->setBackgroundMode( QWidget::PaletteBase );
  install_loc_edit->setFontPropagation( QWidget::NoChildren );
  install_loc_edit->setPalettePropagation( QWidget::NoChildren );
  install_loc_edit->setText( "" );
  install_loc_edit->setMaxLength( 32767 );
  install_loc_edit->setEchoMode( QLineEdit::Normal );
  install_loc_edit->setFrame( TRUE );
  
  install_loc_label = new QLabel( this, "install_loc_label" );
  install_loc_label->setGeometry( 350, 280, 140, 20 );
  install_loc_label->setMinimumSize( 0, 0 );
  install_loc_label->setMaximumSize( 32767, 32767 );
  install_loc_label->setFocusPolicy( QWidget::NoFocus );
  install_loc_label->setBackgroundMode( QWidget::PaletteBackground );
  install_loc_label->setFontPropagation( QWidget::NoChildren );
  install_loc_label->setPalettePropagation( QWidget::NoChildren );
  install_loc_label->setText(i18n("Installdir + Filename:") );
  install_loc_label->setAlignment( 289 );
  install_loc_label->setMargin( -1 );
  
  size_label = new QLabel( this, "size_label" );
  size_label->setGeometry( 350, 80, 60, 30 );
  size_label->setMinimumSize( 0, 0 );
  size_label->setMaximumSize( 32767, 32767 );
  size_label->setFocusPolicy( QWidget::NoFocus );
  size_label->setBackgroundMode( QWidget::PaletteBackground );
  size_label->setFontPropagation( QWidget::NoChildren );
  size_label->setPalettePropagation( QWidget::NoChildren );
  size_label->setText(i18n("Size:") );
  size_label->setAlignment( 289 );
  size_label->setMargin( -1 );
  
  name_label = new QLabel( this, "name_label" );
  name_label->setGeometry( 350, 50, 60, 30 );
  name_label->setMinimumSize( 0, 0 );
  name_label->setMaximumSize( 32767, 32767 );
  name_label->setFocusPolicy( QWidget::NoFocus );
  name_label->setBackgroundMode( QWidget::PaletteBackground );
  name_label->setFontPropagation( QWidget::NoChildren );
  name_label->setPalettePropagation( QWidget::NoChildren );
  name_label->setText(i18n("Name:") );
  name_label->setAlignment( 289 );
  name_label->setMargin( -1 );
  
  incdist_check = new QCheckBox( this, "incdist_check" );
  incdist_check->setGeometry( 350, 160, 150, 30 );
  incdist_check->setMinimumSize( 0, 0 );
  incdist_check->setMaximumSize( 32767, 32767 );
  incdist_check->setFocusPolicy( QWidget::TabFocus );
  incdist_check->setBackgroundMode( QWidget::PaletteBackground );
  incdist_check->setFontPropagation( QWidget::NoChildren );
  incdist_check->setPalettePropagation( QWidget::NoChildren );
  incdist_check->setText(i18n("include in distribution") );
  incdist_check->setAutoRepeat( FALSE );
  incdist_check->setAutoResize( FALSE );
  
  install_check = new QCheckBox( this, "install_check" );
  install_check->setGeometry( 350, 240, 100, 30 );
  install_check->setMinimumSize( 0, 0 );
  install_check->setMaximumSize( 32767, 32767 );
  install_check->setFocusPolicy( QWidget::TabFocus );
  install_check->setBackgroundMode( QWidget::PaletteBackground );
  install_check->setFontPropagation( QWidget::NoChildren );
  install_check->setPalettePropagation( QWidget::NoChildren );
  install_check->setText(i18n("install") );
  install_check->setAutoRepeat( FALSE );
  install_check->setAutoResize( FALSE );
  
  ok_button = new QPushButton( this, "ok_button" );
  ok_button->setGeometry( 190, 360, 110, 30 );
  ok_button->setMinimumSize( 0, 0 );
  ok_button->setMaximumSize( 32767, 32767 );
  ok_button->setFocusPolicy( QWidget::TabFocus );
  ok_button->setBackgroundMode( QWidget::PaletteBackground );
  ok_button->setFontPropagation( QWidget::NoChildren );
  ok_button->setPalettePropagation( QWidget::NoChildren );
  ok_button->setText( "OK" );
  ok_button->setAutoRepeat( FALSE );
  ok_button->setAutoResize( FALSE );

  cancel_button = new QPushButton( this, "cancel_button" );
  cancel_button->setGeometry( 380, 360, 100, 30 );
  cancel_button->setMinimumSize( 0, 0 );
  cancel_button->setMaximumSize( 32767, 32767 );
  cancel_button->setFocusPolicy( QWidget::TabFocus );
  cancel_button->setBackgroundMode( QWidget::PaletteBackground );
  cancel_button->setFontPropagation( QWidget::NoChildren );
  cancel_button->setPalettePropagation( QWidget::NoChildren );
  cancel_button->setText(i18n("Cancel") );
  cancel_button->setAutoRepeat( FALSE );
  cancel_button->setAutoResize( FALSE );
  
  name_e_label = new QLabel( this, "name_e_label" );
  name_e_label->setGeometry( 410, 50, 100, 30 );
  name_e_label->setMinimumSize( 0, 0 );
  name_e_label->setMaximumSize( 32767, 32767 );
  name_e_label->setFocusPolicy( QWidget::NoFocus );
  name_e_label->setBackgroundMode( QWidget::PaletteBackground );
  name_e_label->setFontPropagation( QWidget::NoChildren );
  name_e_label->setPalettePropagation( QWidget::NoChildren );
  name_e_label->setText(i18n("Untitled") );
  name_e_label->setAlignment( 289 );
  name_e_label->setMargin( -1 );
  
  size_e_label = new QLabel( this, "size_e_label" );
  size_e_label->setGeometry( 410, 80, 70, 30 );
  size_e_label->setMinimumSize( 0, 0 );
  size_e_label->setMaximumSize( 32767, 32767 );
  size_e_label->setFocusPolicy( QWidget::NoFocus );
  size_e_label->setBackgroundMode( QWidget::PaletteBackground );
  size_e_label->setFontPropagation( QWidget::NoChildren );
  size_e_label->setPalettePropagation( QWidget::NoChildren );
  size_e_label->setText( "0" );
  size_e_label->setAlignment( 289 );
  size_e_label->setMargin( -1 );
  
  distribution_group->insert( incdist_check );
  installion_group->insert( install_check );

  resize( 670,414 );
  setMinimumSize( 0, 0 );
  setMaximumSize( 32767, 32767 );
  
  
  KQuickHelp::add(distribution_group, 
		  KQuickHelp::add(incdist_check, i18n("This option sets if the actual file\n"
						      "will be included in the final distri-\n"
						      "bution.")));
  
  KQuickHelp::add(installion_group,
		  KQuickHelp::add(install_loc_label,
				  KQuickHelp::add(install_loc_edit,
						  KQuickHelp::add(install_check, i18n("This option sets if the actual file\n"
										      "will be installed to the platform during\n"
										      "the installation process of make install\n"
										      "If the install button is checked, you have\n"
										      "to set the installation path and filename\n"
										      "Possible values are e.g.:\n"
										      "$(kde_icondir)/filename\n"
										      "$(kde_appsdir)/filename\n"
										      "Other values can be found in the Makefile")))));
  
  
  // fill the list
  QStrList filenames;
  prj->getAllFiles(filenames);
  file_list = new QList<TFileInfo>;
  QString str;
  TFileInfo* info;
  for(str = filenames.first();str!=0;str = filenames.next()){
      info = new TFileInfo;
      *info = prj->getFileInfo(str);
    file_list->append(info);
  }
  connect(log_tree,SIGNAL(selectionChanged(QListViewItem*)),SLOT(slotSelectionChanged(QListViewItem*)));
  connect(install_check,SIGNAL(toggled(bool)),SLOT(slotInstallCheckToogled(bool)));
  connect(ok_button,SIGNAL(clicked()),SLOT(slotOk()));
  connect(cancel_button,SIGNAL(clicked()),SLOT(reject()));
  saved_info = 0;

  
  QListViewItem* selecteditem = log_tree->currentItem();
  slotSelectionChanged(selecteditem);
  

  log_tree->setPopupMenusDisabled(); // disabled menus in the logtree
}
CFilePropDlg::~CFilePropDlg(){
}
/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/ 
void CFilePropDlg::slotSelectionChanged(QListViewItem* item){


  QString str;
  TFileInfo* info;
  if (item->childCount() != 0) return; // no action
  
  // save the old-one
  if(saved_info !=0 ){ //ok,there is a old one,it !=0
    saved_info->type = prj->getTypeFromString( type_combo->currentText() ); 
    saved_info->dist = incdist_check->isChecked();
    saved_info->install = install_check->isChecked();
    saved_info->install_location = install_loc_edit->text();
    KDEBUG(KDEBUG_INFO,DIALOG,"fileinfo  saved");
  }
  // prepare the new 
  // find the tfileinfo
  str = item->text(0);
  for(info = file_list->first();info!=0;info = file_list->next()){
    if (info->rel_name == str){
      break;
    }
  }
  if (info==0){ // not found
    return;
  }
  
  QString filename = prj->getProjectDir() + str;
  QFileInfo file_info(filename);
  QString text;
  name_e_label->setText(file_info.fileName());
  text.setNum(file_info.size());
  size_e_label->setText(text);

  if(info->type != PO){
    incdist_check->setEnabled(true);
    install_loc_edit->setEnabled(true);
    install_check->setEnabled(true);
    install_loc_label->setEnabled(true);
  }
  if (info->type == CPP_HEADER){
    type_combo->setCurrentItem(0);
  }
  if (info->type == CPP_SOURCE){
    type_combo->setCurrentItem(1);
  }
  if (info->type == SCRIPT){
    type_combo->setCurrentItem(2);
  }
  if (info->type == DATA){
    type_combo->setCurrentItem(3);
  }
  if (info->type == PO){
    type_combo->setCurrentItem(4);
  }

  if(info->dist){
    incdist_check->setChecked(true);
  }
  else{
    incdist_check->setChecked(false);
  }
  
  if(info->install){
    install_check->setChecked(true);
    install_loc_edit->setEnabled(true);
  }
  else{
    install_check->setChecked(false);
    install_loc_edit->setEnabled(false);
  }
  install_loc_edit->setText(info->install_location);
  // save the current
  cerr << "SAVE=INFO";
  saved_info = info;
  
  if (info->type == "PO"){
    incdist_check->setEnabled(false);
    install_loc_edit->setEnabled(false);
    install_check->setEnabled(false);
    install_loc_label->setEnabled(false);
  }
}
void CFilePropDlg::slotInstallCheckToogled(bool on){
    install_loc_edit->setEnabled(on);
    if(on){
	if(QString(install_loc_edit->text()).isEmpty()){
	    install_loc_edit->setText(
				      (name_e_label->text()) != 0 ?name_e_label->text():"");
	}
    }
}
void CFilePropDlg::slotOk(){
    if(saved_info !=0 ){ //ok,there is a old one,it !=0
	saved_info->type = prj->getTypeFromString( type_combo->currentText() );
	saved_info->dist = incdist_check->isChecked();
	saved_info->install = install_check->isChecked();
	saved_info->install_location = install_loc_edit->text();
    }
    
    TFileInfo* info;
    for(info = file_list->first();info !=0;info = file_list->next()){
	prj->writeFileInfo(*info);
    }
    prj->updateMakefilesAm();
    accept();
    
}
