/***************************************************************************
                     cnewfiledlg.cpp - the new file dialog
                             -------------------                                         

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
  setCaption(i18n("New File..."));    
	// the tabview
	tab = new KTabCtl(this);
  tab->setGeometry( 20, 20, 230, 160);
  tab->setBorder(false);

  list_linux = new QListBox( tab, "list_linux" );
  list_linux->insertItem(i18n("lsm File - Linux Software Map"));
  list_linux->insertItem(i18n("kdelnk-File - for the KDE-Menu"));
  list_linux->insertItem(i18n("Icon (*.xpm)"));
  list_linux->setMultiSelection( FALSE );
  list_linux->setCurrentItem(0);

  list_manuals = new QListBox( tab, "list_manuals" );
  list_manuals->insertItem(i18n("english (.sgml)"));
  list_manuals->setMultiSelection( FALSE );
  list_manuals->setCurrentItem(0);

  list_cpp = new QListBox( tab, "list_cpp" );
  list_cpp->insertItem(i18n("C/C++ Header (*.h,*.hxx)"));
  list_cpp->insertItem(i18n("C/C++ File (*.cpp,*.c,*.cc,*.C ...)"));
  list_cpp->insertItem(i18n("Empty Textfile"));
  list_cpp->setMultiSelection( FALSE );
  list_cpp->setCurrentItem(0);

  tab->addTab(list_cpp,i18n("General"));
  tab->addTab(list_manuals,i18n("Manuals"));
  tab->addTab(list_linux,i18n("Linux/KDE"));

	label_filename = new QLabel( this, "label_filename" );
	label_filename->setGeometry( 270, 30, 170, 30 );
	label_filename->setMinimumSize( 10, 10 );
	label_filename->setMaximumSize( 32767, 32767 );
	label_filename->setFocusPolicy( QWidget::NoFocus );
	label_filename->setBackgroundMode( QWidget::PaletteBackground );
	label_filename->setFontPropagation( QWidget::NoChildren );
	label_filename->setPalettePropagation( QWidget::NoChildren );
	label_filename->setText( i18n("Filename:") );
	label_filename->setAlignment( 289 );
	label_filename->setMargin( -1 );
	
	edit = new QLineEdit( this, "edit" );
	edit->setGeometry( 270, 70, 180, 30 );
	edit->setMinimumSize( 10, 10 );
	edit->setMaximumSize( 32767, 32767 );
	edit->setFocusPolicy( QWidget::StrongFocus );
	edit->setBackgroundMode( QWidget::PaletteBase );
	edit->setFontPropagation( QWidget::NoChildren );
	edit->setPalettePropagation( QWidget::NoChildren );
	edit->setText( "" );
	edit->setMaxLength( 32767 );
	edit->setEchoMode( QLineEdit::Normal );
	edit->setFrame( TRUE );

	check_use_template = new QCheckBox( this, "check_use_template" );
	check_use_template->setGeometry( 270, 120, 180, 30 );
	check_use_template->setMinimumSize( 10, 10 );
	check_use_template->setMaximumSize( 32767, 32767 );
	check_use_template->setFocusPolicy( QWidget::TabFocus );
	check_use_template->setBackgroundMode( QWidget::PaletteBackground );
	check_use_template->setFontPropagation( QWidget::NoChildren );
	check_use_template->setPalettePropagation( QWidget::NoChildren );
	check_use_template->setText(i18n("use Template") );
	check_use_template->setAutoRepeat( FALSE );
	check_use_template->setAutoResize( FALSE );
	
	button_group = new QButtonGroup( this, "button_group" );
	button_group->setGeometry( 20, 200, 430, 140 );
	button_group->setMinimumSize( 10, 10 );
	button_group->setMaximumSize( 32767, 32767 );
	button_group->setFocusPolicy( QWidget::NoFocus );
	button_group->setBackgroundMode( QWidget::PaletteBackground );
	button_group->setFontPropagation( QWidget::NoChildren );
	button_group->setPalettePropagation( QWidget::NoChildren );
	button_group->setFrameStyle( 49 );
	button_group->setTitle( i18n("Project Options" ));
	button_group->setAlignment( 1 );

	check_add_project = new QCheckBox( this, "check_add_project" );
	check_add_project->setGeometry( 30, 220, 210, 30 );
	check_add_project->setMinimumSize( 10, 10 );
	check_add_project->setMaximumSize( 32767, 32767 );
	check_add_project->setFocusPolicy( QWidget::TabFocus );
	check_add_project->setBackgroundMode( QWidget::PaletteBackground );
	check_add_project->setFontPropagation( QWidget::NoChildren );
	check_add_project->setPalettePropagation( QWidget::NoChildren );
	check_add_project->setText(i18n("add to Project") );
	check_add_project->setAutoRepeat( FALSE );
	check_add_project->setAutoResize( FALSE );

	location_label = new QLabel( this, "location_label" );
	location_label->setGeometry( 30, 250, 160, 30 );
	location_label->setMinimumSize( 0, 0 );
	location_label->setMaximumSize( 32767, 32767 );
	location_label->setFocusPolicy( QWidget::NoFocus );
	location_label->setBackgroundMode( QWidget::PaletteBackground );
	location_label->setFontPropagation( QWidget::NoChildren );
	location_label->setPalettePropagation( QWidget::NoChildren );
	location_label->setText(i18n("Location:") );
	location_label->setAlignment( 289 );
	location_label->setMargin( -1 );
	
	prj_loc_edit = new QLineEdit( this, "prj_loc_edit" );
	prj_loc_edit->setGeometry( 30, 290, 360, 30 );
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
	loc_button->setGeometry( 410, 290, 30, 30 );
	loc_button->setMinimumSize( 0, 0 );
	loc_button->setMaximumSize( 32767, 32767 );
	loc_button->setFocusPolicy( QWidget::TabFocus );
	loc_button->setBackgroundMode( QWidget::PaletteBackground );
	loc_button->setFontPropagation( QWidget::NoChildren );
	loc_button->setPalettePropagation( QWidget::NoChildren );
	QPixmap pix;
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
	loc_button->setPixmap(pix);
	loc_button->setAutoRepeat( FALSE );
	loc_button->setAutoResize( FALSE );

	button_group->insert( check_add_project );
	button_group->insert( loc_button );

	ok = new QPushButton( this, "ok" );
	ok->setGeometry( 110, 360, 100, 30 );
	ok->setMinimumSize( 10, 10 );
	ok->setMaximumSize( 32767, 32767 );
	ok->setFocusPolicy( QWidget::TabFocus );
	ok->setBackgroundMode( QWidget::PaletteBackground );
	ok->setFontPropagation( QWidget::NoChildren );
	ok->setPalettePropagation( QWidget::NoChildren );
	ok->setText(i18n("OK"));
	ok->setAutoRepeat( FALSE );
	ok->setAutoResize( FALSE );
	ok->setDefault( TRUE );
	
	cancel = new QPushButton( this, "cancel" );
	cancel->setGeometry( 270, 360, 100, 30 );
	cancel->setMinimumSize( 10, 10 );
	cancel->setMaximumSize( 32767, 32767 );
	cancel->setFocusPolicy( QWidget::TabFocus );
	cancel->setBackgroundMode( QWidget::PaletteBackground );
	cancel->setFontPropagation( QWidget::NoChildren );
	cancel->setPalettePropagation( QWidget::NoChildren );
	cancel->setText(i18n("Cancel") );
	cancel->setAutoRepeat( FALSE );
	cancel->setAutoResize( FALSE );

  setMinimumSize( 0, 0 );
  setMaximumSize( 32767, 32767 );
  resize( 470,410 );

  connect(tab,SIGNAL(tabSelected(int)),SLOT(slotTabSelected(int)));
  connect(ok,SIGNAL(clicked()),SLOT(slotOKClicked()));
  connect(cancel,SIGNAL(clicked()),SLOT(reject()));
  connect(loc_button,SIGNAL(clicked()),SLOT(slotLocButtonClicked()));
  connect(check_add_project,SIGNAL(clicked()),SLOT(slotAddToProject()));
  connect(edit,SIGNAL(textChanged (const char *)),SLOT(slotEditTextChanged(const char*)));

  connect(list_linux,SIGNAL(highlighted(int)),SLOT(slotListHighlighted(int)));
  connect(list_cpp,SIGNAL(highlighted(int)),SLOT(slotListHighlighted(int)));
  connect(list_manuals,SIGNAL(highlighted(int)),SLOT(slotListHighlighted(int)));
  //always default, add to Project
  check_add_project->setChecked(true);

  autocompletion = true;
}

void CNewFileDlg::slotTabSelected(int item){
  if(current == 0){ //cpp
    slotListHighlighted(list_cpp->currentItem());
  }
  if(current == 1){ // manuals
    slotListHighlighted(list_manuals->currentItem());
  }
  if(current == 2){ // linux
    slotListHighlighted(list_linux->currentItem());
  }
  current = item;
}
void CNewFileDlg::slotOKClicked(){
  QString text = edit->text();
  if ( (fileType() == "CPP") && 
       !(text.right(4) == ".cpp" || text.right(3) == ".cc" 
	|| text.right(2) == ".C" || text.right(2) == ".c" || text.right(4) == ".cxx" || text.right(3) == ".ec" || text.right(5) == ".ecpp" )){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .cpp,.c,.cc,.C,.cxx,.ec or .ecpp!")
		     ,KMsgBox::EXCLAMATION);
    return;
  }
  if ( (fileType() == "HEADER") && !(text.right(2) == ".h" || (text.right(4) == ".hxx"))){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .h or .hxx!"),KMsgBox::EXCLAMATION);
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
  if ( (fileType() == "EN_SGML") && (text.right(5) != ".sgml")){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .sgml !"),KMsgBox::EXCLAMATION);
    return;
  }
  if ( (fileType() == "ICON") && (text.right(4) != ".xpm")){
    KMsgBox::message(this,i18n("Error..."),i18n("The filename must end with .xpm !"),KMsgBox::EXCLAMATION);
    return;
  }
  if (text.isEmpty()){
    KMsgBox::message(this,i18n("Error..."),i18n("You must enter a filename!"),KMsgBox::EXCLAMATION);
    return;
  }
  if(addToProject() == true && (location().contains(prj->getProjectDir())) == 0 ){
    KMsgBox::message(this,i18n("Error..."),i18n("You must choose a location,that is in your project-dir!")
		     ,KMsgBox::EXCLAMATION);
    return;
  }
  QString filename = fileName();
  QString complete_filename;
  // contruct the complete_filename
  complete_filename = location() + filename;
  
  if(QFile::exists(complete_filename)){
    if(KMsgBox::yesNo(0,i18n("Files exists!"),
		      i18n("You have added a file to the project that already exists.\nDo you want overwrite the old one?")) == 2){
      return;
    }
  }
  
  /*************----------generate the new File----------****************/
  
  QString section;
  QString type;
  QString filetype = fileType(); 
  CGenerateNewFile generator;
 
   
   
   // check if generate a empty file or generate one
  if (useTemplate() && (filetype != "TEXTFILE") && filetype != "ICON"){ // generate one,textfile always empty
    
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
   }
  else { // no template, -> empty file or icon
    if(filetype == "ICON"){
      generator.genIcon(complete_filename);
    }
    else{
      
      QFile file(complete_filename);
      file.remove();
      file.open(IO_ReadWrite);
      file.close();
    }
  }
  accept();
}

QString CNewFileDlg::fileName(){
  return edit->text();
}
QString CNewFileDlg::fileType(){ 
  QString str;
  if (current == 0){ // cpp/header
    str = list_cpp->text(list_cpp->currentItem());
    if (str == i18n("C/C++ File (*.cpp,*.c,*.cc,*.C ...)")){
      return "CPP";
    }
    if (str == i18n("C/C++ Header (*.h,*.hxx)")){
      return "HEADER";
    }
    if (str == i18n("Empty Textfile")){
      return "TEXTFILE";
    }
  }
  
  
  if (current == 1){ // manuals
    str = list_manuals->text(list_manuals->currentItem());
    if (str == i18n("english (.sgml)")){
      return "EN_SGML";
    }
  }
  if (current == 2){ // /linux/kde
    str = list_linux->text(list_linux->currentItem());
    if (str == i18n("kdelnk-File - for the KDE-Menu")){
      return "KDELNK";
    }
    if (str == i18n("lsm File - Linux Software Map")){
      return "LSM";
    }
    if (str == i18n("Icon (*.xpm)")){
      return "ICON";
    }
  }
  return "TEST";
}
bool CNewFileDlg::useTemplate(){
  return check_use_template->isChecked();
}
bool CNewFileDlg::addToProject(){
  return check_add_project->isChecked();
}
void CNewFileDlg::setUseTemplate(){
  check_use_template->setChecked(true);
}
void CNewFileDlg::setAddToProject(){
  check_add_project->setChecked(true);
}
void CNewFileDlg::slotLocButtonClicked(){
  QString str=  KDirDialog::getDirectory(prj_loc_edit->text(),this,"test");
  if(!str.isEmpty()){
    prj_loc_edit->setText(str);
  }
}
QString CNewFileDlg::location(){
	QString str = prj_loc_edit->text();
	if (str.right(1) != "/"){
	 	str = str + "/";
	}
  return str;
}
void CNewFileDlg::slotAddToProject(){
  if(check_add_project->isChecked()){
    prj_loc_edit->setEnabled(true);
    loc_button->setEnabled(true);
  }
  else{
    prj_loc_edit->setEnabled(false);
    loc_button->setEnabled(false);
  }
}

void CNewFileDlg::slotEditTextChanged(const char* text){
  QString filetype = fileType();
  if(autocompletion){

    if (filetype != "TEXTFILE" ) {
      autocompletion = false;
      
      if (filetype == "HEADER" ) {
	edit->setText(text + QString(".h"));
      }
      if (filetype == "CPP" ) {
	edit->setText(text + QString(".cpp"));
      }
      if (filetype == "KDELNK" ) {
	edit->setText(text + QString(".kdelnk"));
      }
      if (filetype == "EN_SGML" ) {
	edit->setText(text + QString(".sgml"));
      }
      if (filetype == "LSM" ) {
	edit->setText(text + QString(".lsm"));
      }
      if (filetype == "ICON" ) {
      edit->setText(text + QString(".xpm"));
      }
      if (filetype == "KDELNK" ) {
	edit->setText(text + QString(".kdelnk"));
      }
      edit->setCursorPosition(1);
    }
    
  }
}
void CNewFileDlg::slotListHighlighted(int){
  edit->clear();
  edit->setFocus();
  autocompletion = true;
}



