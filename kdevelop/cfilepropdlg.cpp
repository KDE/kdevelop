/***************************************************************************
   cfilepropdlg.cpp  -  yet another sourcefile with many confusing hacks,:-( sorry
                             -------------------

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
#include "clogfileview.h"
#include "cproject.h"

#include <klocale.h>
#include "debug.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qgrid.h>
#include <qlayout.h>
#include <kbuttonbox.h>

#include <iostream>
using namespace std;

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/
CFilePropDlg::CFilePropDlg(QWidget *parent, const char *name,CProject* prj, const QString& preselecteditem )
        : QDialog(parent,name,true)
{
  this->prj = prj; // save the pointer
  setCaption(i18n("File Properties"));

  QGridLayout *grid1 = new QGridLayout(this,4,2,15,7);

  distribution_group = new QButtonGroup( this, "distribution_group" );
  distribution_group->setFocusPolicy( QWidget::NoFocus );
  distribution_group->setBackgroundMode( QWidget::PaletteBackground );
  distribution_group->setFrameStyle( 49 );
  distribution_group->setTitle(i18n("Distribution") );
  distribution_group->setAlignment( 1 );



  QVBoxLayout *lay1= new QVBoxLayout( distribution_group );
  lay1->setMargin( 15 );
  lay1->setSpacing( 10 );
  incdist_check = new QCheckBox( distribution_group, "incdist_check" );
  incdist_check->setFocusPolicy( QWidget::TabFocus );
  incdist_check->setBackgroundMode( QWidget::PaletteBackground );
  incdist_check->setText(i18n("Include in distribution") );
  incdist_check->setAutoRepeat( FALSE );
  incdist_check->setAutoResize( FALSE );
  lay1->addWidget(incdist_check);

  grid1->addMultiCellWidget(distribution_group,1,1, 1,2);

  type_group = new QButtonGroup( this, "type_group" );
  type_group->setFocusPolicy( QWidget::NoFocus );
  type_group->setBackgroundMode( QWidget::PaletteBackground );
  type_group->setFrameStyle( 49 );
  type_group->setTitle(i18n("Filetype") );
  type_group->setAlignment( 1 );

  lay1 = new QVBoxLayout( type_group );
  lay1->setMargin( 15 );
  lay1->setSpacing( 10 );
  type_combo = new QComboBox( FALSE, type_group, "type_combo" );
  type_combo->setFocusPolicy( QWidget::StrongFocus );
  type_combo->setBackgroundMode( QWidget::PaletteBackground );
  type_combo->setSizeLimit( 10 );
  type_combo->setAutoResize( FALSE );
  type_combo->insertItem( "HEADER" );
  type_combo->insertItem( "SOURCE" );
  type_combo->insertItem( "SCRIPT" );
  type_combo->insertItem( "DATA" );
  type_combo->insertItem( "PO" );
  type_combo->insertItem( "QT_TS" );
  type_combo->insertItem( "KDEV_DIALOG" );
  lay1->addWidget(type_combo);
  grid1->addWidget(type_group,0,2);


  installion_group = new QButtonGroup( this, "installion_group" );
  installion_group->setFocusPolicy( QWidget::NoFocus );
  installion_group->setBackgroundMode( QWidget::PaletteBackground );
  installion_group->setFrameStyle( 49 );
  installion_group->setTitle(i18n("Installation") );
  installion_group->setAlignment( 1 );

  lay1= new QVBoxLayout( installion_group );
  lay1->setMargin( 15 );
  lay1->setSpacing( 10 );
  install_check = new QCheckBox( installion_group, "install_check" );
  install_check->setFocusPolicy( QWidget::TabFocus );
  install_check->setBackgroundMode( QWidget::PaletteBackground );
  install_check->setText(i18n("Install") );
  install_check->setAutoRepeat( FALSE );
  install_check->setAutoResize( FALSE );
  lay1->addWidget(install_check);

  install_loc_label = new QLabel(installion_group, "install_loc_label" );
  install_loc_label->setFocusPolicy( QWidget::NoFocus );
  install_loc_label->setBackgroundMode( QWidget::PaletteBackground );
  install_loc_label->setText(i18n("Installdir + Filename:") );
  install_loc_label->setAlignment( 289 );
  install_loc_label->setMargin( -1 );
  lay1->addWidget(install_loc_label);

  install_loc_edit = new QLineEdit( installion_group, "install_loc_edit" );
  install_loc_edit->setFocusPolicy( QWidget::StrongFocus );
  install_loc_edit->setBackgroundMode( QWidget::PaletteBase );
  install_loc_edit->setText( "" );
  install_loc_edit->setMaxLength( 32767 );
  install_loc_edit->setEchoMode( QLineEdit::Normal );
  install_loc_edit->setFrame( TRUE );
  lay1->addWidget(install_loc_edit);
  grid1->addMultiCellWidget(installion_group,2,2, 1,2);


  file_group = new QButtonGroup( this, "file_group" );
  file_group->setFocusPolicy( QWidget::NoFocus );
  file_group->setBackgroundMode( QWidget::PaletteBackground );
  file_group->setFrameStyle( 49 );
  file_group->setTitle(i18n("File") );
  file_group->setAlignment( 1 );

  QGridLayout *grid2 = new QGridLayout(file_group,2,2,15,7);
  size_label = new QLabel( file_group, "size_label" );
  size_label->setFocusPolicy( QWidget::NoFocus );
  size_label->setBackgroundMode( QWidget::PaletteBackground );
  size_label->setText(i18n("Size:") );
  size_label->setAlignment( 289 );
  size_label->setMargin( -1 );
  grid2->addWidget(size_label,1,0);

  name_label = new QLabel( file_group, "name_label" );
  name_label->setFocusPolicy( QWidget::NoFocus );
  name_label->setBackgroundMode( QWidget::PaletteBackground );
  name_label->setText(i18n("Name:") );
  name_label->setAlignment( 289 );
  name_label->setMargin( -1 );
  grid2->addWidget(name_label,0,0);


  name_e_label = new QLabel( file_group, "name_e_label" );
  name_e_label->setFocusPolicy( QWidget::NoFocus );
  name_e_label->setBackgroundMode( QWidget::PaletteBackground );
  name_e_label->setText(i18n("Untitled") );
  name_e_label->setAlignment( 289 );
  name_e_label->setMargin( -1 );
  grid2->addWidget(name_e_label,0,1);

  size_e_label = new QLabel( file_group, "size_e_label" );
  size_e_label->setFocusPolicy( QWidget::NoFocus );
  size_e_label->setBackgroundMode( QWidget::PaletteBackground );
  size_e_label->setText( "0" );
  size_e_label->setAlignment( 289 );
  size_e_label->setMargin( -1 );
  grid2->addWidget(size_e_label,1,1);

  grid1->addWidget(file_group,0,1);

  log_tree = new CLogFileView(true, this, "log_tree");
  grid1->addMultiCellWidget(log_tree,0,2, 0,0);
  if(preselecteditem != ""){
    log_tree->setPreSelectedItem(preselecteditem);
  }
  else{
    log_tree->setFirstItemSelected(); // select the first item
      }
  log_tree->setAllGroupsOpened();
  log_tree->refresh(prj);


  KButtonBox *bb = new KButtonBox( this );
   bb->addStretch();
  ok_button = bb->addButton( i18n("OK") );
  ok_button->setDefault( TRUE );
  ok_button->setBackgroundMode( QWidget::PaletteBackground );
  ok_button->setAutoRepeat( FALSE );
  ok_button->setAutoResize( FALSE );
  cancel_button = bb->addButton( i18n( "Cancel" ) );
  cancel_button->setFocusPolicy( QWidget::TabFocus );
  cancel_button->setBackgroundMode( QWidget::PaletteBackground );
  cancel_button->setAutoRepeat( FALSE );
  cancel_button->setAutoResize( FALSE );
  bb->layout();
  grid1->addWidget(bb,3,2);


  QString incdist_checkMsg = i18n("This option sets if the actual file\n"
						      "will be included in the final distri-\n"
						      "bution.");
  QWhatsThis::add(distribution_group, incdist_checkMsg);
  QWhatsThis::add(incdist_check, incdist_checkMsg);

  QString install_checkMsg = i18n("This option sets if the actual file\n"
										      "will be installed to the platform during\n"
										      "the installation process of make install\n"
										      "If the install button is checked, you have\n"
										      "to set the installation path and filename\n"
										      "Possible values are e.g.:\n"
										      "$(kde_icondir)/filename\n"
										      "$(kde_appsdir)/filename\n"
										      "Other values can be found in the Makefile");
  QWhatsThis::add(installion_group, install_checkMsg);
  QWhatsThis::add(install_loc_label, install_checkMsg);
  QWhatsThis::add(install_loc_edit, install_checkMsg);
  QWhatsThis::add(install_check, install_checkMsg);

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
  connect(type_combo,SIGNAL(activated(int)),SLOT(slotTypeComboActivated(int)));
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
void CFilePropDlg::slotTypeComboActivated(int index){
    if(index != 4){ // PO
	if(index == 1){ // CPP_SOURCE
	    incdist_check->setEnabled(false);
	}
	else{
	    incdist_check->setEnabled(true);
	}
	
	install_loc_edit->setEnabled(true);
	install_check->setEnabled(true);
	install_loc_label->setEnabled(true);
	
    }
    else {
	incdist_check->setEnabled(false);
	install_loc_edit->setEnabled(false);
	install_check->setEnabled(false);
	install_loc_label->setEnabled(false);
	
    }
}
void CFilePropDlg::slotSelectionChanged(QListViewItem* item){


  QString str;
  TFileInfo* info;
  if (item == 0 || item->childCount() != 0)
    return; // no action
  
  // save the old-one
  if(saved_info !=0 ){ //ok,there is a old one,it !=0
    saved_info->type = prj->getTypeFromString( type_combo->currentText() ); 
    if(QString(type_combo->currentText()) == "HEADER"){
      saved_info->type = CPP_HEADER;
    }
    if(QString(type_combo->currentText()) == "SOURCE"){
      saved_info->type = CPP_SOURCE;
    }  
    
    saved_info->dist = incdist_check->isChecked();
    saved_info->install = install_check->isChecked();
    saved_info->install_location = install_loc_edit->text();
//    KDEBUG(KDEBUG_INFO,DIALOG,"fileinfo  saved");
  }
  // prepare the new 
  // find the tfileinfo
  str = log_tree->getFileName(item);
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
    if(info->type == CPP_SOURCE){
      incdist_check->setEnabled(false);
    }
    else{
      incdist_check->setEnabled(true);
    }
   
    install_loc_edit->setEnabled(true);
    install_check->setEnabled(true);
    install_loc_label->setEnabled(true);
  }
  else{
    incdist_check->setEnabled(false);
    install_loc_edit->setEnabled(false);
    install_check->setEnabled(false);
    install_loc_label->setEnabled(false);
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
  if (info->type == QT_TS){
    type_combo->setCurrentItem(5);
  }
  if (info->type == KDEV_DIALOG){
    type_combo->setCurrentItem(6);
  }

  if(info->dist || info->type == CPP_SOURCE){
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
  //  cerr << "SAVE=INFO";
  saved_info = info;
  
}
void CFilePropDlg::slotInstallCheckToogled(bool on){
  install_loc_edit->setEnabled(on);
  if(on)
  {
    if(QString(install_loc_edit->text()).isEmpty())
    {
      if ((name_e_label->text()).isEmpty())
        install_loc_edit->setText(name_e_label->text());
      else
        install_loc_edit->setText(QString::null);
    }
  }
}
void CFilePropDlg::slotOk(){
  if(saved_info !=0 ){ //ok,there is a old one,it !=0
    saved_info->type = prj->getTypeFromString( type_combo->currentText()); 
    if(QString(type_combo->currentText()) == "HEADER"){
      saved_info->type = CPP_HEADER;
    }
    if(QString(type_combo->currentText()) == "SOURCE"){
      saved_info->type = CPP_SOURCE;
    }  
    
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



#include "cfilepropdlg.moc"
